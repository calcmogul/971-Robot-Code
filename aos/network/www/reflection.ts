// This library provides a few basic reflection utilities for Flatbuffers.
// Currently, this only really supports the level of reflection that would
// be necessary to convert a serialized flatbuffer to JSON using just a
// reflection.Schema flatbuffer describing the type.
// The current implementation is also not necessarily robust to invalidly
// constructed flatbuffers.
// See reflection_test_main.ts for sample usage.

import {BaseType, Schema, Object, Field} from 'aos/configuration_generated';

// Returns the size, in bytes, of the given type. For vectors/strings/etc.
// returns the size of the offset.
function typeSize(baseType: BaseType): number {
  switch (baseType) {
    case BaseType.None:
    case BaseType.UType:
    case BaseType.Bool:
    case BaseType.Byte:
    case BaseType.UByte:
      return 1;
    case BaseType.Short:
    case BaseType.UShort:
      return 2;
    case BaseType.Int:
    case BaseType.UInt:
      return 4;
    case BaseType.Long:
    case BaseType.ULong:
      return 8;
    case BaseType.Float:
      return 4;
    case BaseType.Double:
      return 8;
    case BaseType.String:
    case BaseType.Vector:
    case BaseType.Obj:
    case BaseType.Union:
    case BaseType.Array:
      return 4;
  }
}

// Returns whether the given type is a scalar type.
function isScalar(baseType: BaseType): boolean {
  switch (baseType) {
    case BaseType.UType:
    case BaseType.Bool:
    case BaseType.Byte:
    case BaseType.UByte:
    case BaseType.Short:
    case BaseType.UShort:
    case BaseType.Int:
    case BaseType.UInt:
    case BaseType.Long:
    case BaseType.ULong:
    case BaseType.Float:
    case BaseType.Double:
      return true;
    case BaseType.None:
    case BaseType.String:
    case BaseType.Vector:
    case BaseType.Obj:
    case BaseType.Union:
    case BaseType.Array:
      return false;
  }
}

// Returns whether the given type is integer or not.
function isInteger(baseType: BaseType): boolean {
  switch (baseType) {
    case BaseType.UType:
    case BaseType.Bool:
    case BaseType.Byte:
    case BaseType.UByte:
    case BaseType.Short:
    case BaseType.UShort:
    case BaseType.Int:
    case BaseType.UInt:
    case BaseType.Long:
    case BaseType.ULong:
      return true;
    case BaseType.Float:
    case BaseType.Double:
    case BaseType.None:
    case BaseType.String:
    case BaseType.Vector:
    case BaseType.Obj:
    case BaseType.Union:
    case BaseType.Array:
      return false;
  }
}

// Returns whether the given type is a long--this is needed to know whether it
// can be represented by the normal javascript number (8-byte integers require a
// special type, since the native number type is an 8-byte double, which won't
// represent 8-byte integers to full precision).
function isLong(baseType: BaseType): boolean {
  return isInteger(baseType) && (typeSize(baseType) > 4);
}

// TODO(james): Use the actual flatbuffers.ByteBuffer object; this is just
// to prevent the typescript compiler from complaining.
class ByteBuffer {}

// Stores the data associated with a Table within a given buffer.
export class Table {
  // Wrapper to represent an object (Table or Struct) within a ByteBuffer.
  // The ByteBuffer is the raw data associated with the object.
  // typeIndex is an index into the schema object vector for the parser
  // object that this is associated with.
  // offset is the absolute location within the buffer where the root of the
  // object is.
  // Note that a given Table assumes that it is being used with a particular
  // Schema object.
  // External users should generally not be using this constructor directly.
  constructor(
      public readonly bb: ByteBuffer,
      public readonly typeIndex: number, public readonly offset: number) {}
  // Constructs a Table object for the root of a ByteBuffer--this assumes that
  // the type of the Table is the root table of the Parser that you are using.
  static getRootTable(bb: ByteBuffer): Table {
    return new Table(bb, -1, bb.readInt32(bb.position()) + bb.position());
  }
  // Reads a scalar of a given type at a given offset.
  readScalar(fieldType: BaseType, offset: number) {
    switch (fieldType) {
      case BaseType.UType:
      case BaseType.Bool:
        return this.bb.readUint8(offset);
      case BaseType.Byte:
        return this.bb.readInt8(offset);
      case BaseType.UByte:
        return this.bb.readUint8(offset);
      case BaseType.Short:
        return this.bb.readInt16(offset);
      case BaseType.UShort:
        return this.bb.readUint16(offset);
      case BaseType.Int:
        return this.bb.readInt32(offset);
      case BaseType.UInt:
        return this.bb.readUint32(offset);
      case BaseType.Long:
        return this.bb.readInt64(offset);
      case BaseType.ULong:
        return this.bb.readUint64(offset);
      case BaseType.Float:
        return this.bb.readFloat32(offset);
      case BaseType.Double:
        return this.bb.readFloat64(offset);
    }
    throw new Error('Unsupported message type ' + baseType);
  }
};

// The Parser class uses a Schema to provide all the utilities required to
// parse flatbuffers that have a type that is the same as the root_type defined
// by the Schema.
// The classical usage would be to, e.g., be reading a channel with a type of
// "aos.FooBar". At startup, you would construct a Parser from the channel's
// Schema. When a message is received on the channel , you would then use
// Table.getRootTable() on the received buffer to construct the Table, and
// then access the members using the various methods of the Parser (or just
// convert the entire object to a javascript Object/JSON using toObject()).
export class Parser {
  constructor(private readonly schema: Schema) {}

  // Parse a Table to a javascript object. This is can be used, e.g., to convert
  // a flatbuffer Table to JSON.
  // If readDefaults is set to true, then scalar fields will be filled out with
  // their default values if not populated; if readDefaults is false and the
  // field is not populated, the resulting object will not populate the field.
  toObject(table: Table, readDefaults: boolean = false) {
    const result = {};
    const schema = this.getType(table.typeIndex);
    const numFields = schema.fieldsLength();
    for (let ii = 0; ii < numFields; ++ii) {
      const field = schema.fields(ii);
      const baseType = field.type().baseType();
      let fieldValue = null;
      if (isScalar(baseType)) {
        fieldValue = this.readScalar(table, field.name(), readDefaults);
      } else if (baseType === BaseType.String) {
        fieldValue = this.readString(table, field.name());
      } else if (baseType === BaseType.Obj) {
        const subTable = this.readTable(table, field.name());
        if (subTable !== null) {
          fieldValue = this.toObject(subTable, readDefaults);
        }
      } else if (baseType === BaseType.Vector) {
        const elementType = field.type().element();
        if (isScalar(elementType)) {
          fieldValue = this.readVectorOfScalars(table, field.name());
        } else if (elementType === BaseType.String) {
          fieldValue = this.readVectorOfStrings(table, field.name());
        } else if (elementType === BaseType.Obj) {
          const tables = this.readVectorOfTables(table, field.name());
          if (tables !== null) {
            fieldValue = [];
            for (const table of tables) {
              fieldValue.push(this.toObject(table, readDefaults));
            }
          }
        } else {
          throw new Error('Vectors of Unions and Arrays are not supported.');
        }
      } else {
        throw new Error(
            'Unions and Arrays are not supported in field ' + field.name());
      }
      if (fieldValue !== null) {
        result[field.name()] = fieldValue;
      }
    }
    return result;
  }

  // Returns the Object definition associated with the given type index.
  getType(typeIndex: number): Object {
    if (typeIndex === -1) {
      return this.schema.rootTable();
    }
    if (typeIndex < 0 || typeIndex > this.schema.objectsLength()) {
      throw new Error("Type index out-of-range.");
    }
    return this.schema.objects(typeIndex);
  }

  // Retrieves the Field schema for the given field name within a given
  // type index.
  getField(fieldName: string, typeIndex: number): Field {
    const schema: Object = this.getType(typeIndex);
    const numFields = schema.fieldsLength();
    for (let ii = 0; ii < numFields; ++ii) {
      const field = schema.fields(ii);
      const name = field.name();
      if (fieldName === name) {
        return field;
      }
    }
    throw new Error(
        'Couldn\'t find field ' + fieldName + ' options are ' + fields);
  }

  // Reads a scalar with the given field name from a Table. If readDefaults
  // is set to false and the field is unset, we will return null. If
  // readDefaults is true and the field is unset, we will look-up the default
  // value for the field and return that.
  // For 64-bit fields, returns a flatbuffer Long rather than a standard number.
  // TODO(james): For this and other accessors, determine if there is a
  // significant performance gain to be had by using readScalar to construct
  // an accessor method rather than having to redo the schema inspection on
  // every call.
  readScalar(table: Table, fieldName: string, readDefaults: boolean = false) {
    const field = this.getField(fieldName, table.typeIndex);
    const fieldType = field.type();
    const isStruct = this.getType(table.typeIndex).isStruct();
    if (!isScalar(fieldType.baseType())) {
      throw new Error('Field ' + fieldName + ' is not a scalar type.');
    }

    if (isStruct) {
      return table.readScalar(
          fieldType.baseType(), table.offset + field.offset());
    }

    const offset =
        table.offset + table.bb.__offset(table.offset, field.offset());
    if (offset === table.offset) {
      if (!readDefaults) {
        return null;
      }
      if (isInteger(fieldType.baseType())) {
        if (isLong(fieldType.baseType())) {
          return field.defaultInteger();
        } else {
          if (field.defaultInteger().high != 0) {
            throw new Error(
                '<=4 byte integer types should not use 64-bit default values.');
          }
          return field.defaultInteger().low;
        }
      } else {
        return field.defaultReal();
      }
    }
    return table.readScalar(fieldType.baseType(), offset);
  }
  // Reads a string with the given field name from the provided Table.
  // If the field is unset, returns null.
  readString(table: Table, fieldName: string): string|null {
    const field = this.getField(fieldName, table.typeIndex);
    const fieldType = field.type();
    if (fieldType.baseType() !== BaseType.String) {
      throw new Error('Field ' + fieldName + ' is not a string.');
    }

    const offsetToOffset =
        table.offset + table.bb.__offset(table.offset, field.offset());
    if (offsetToOffset === table.offset) {
      return null;
    }
    return table.bb.__string(offsetToOffset);
  }
  // Reads a sub-message from the given Table. The sub-message may either be
  // a struct or a Table. Returns null if the sub-message is not set.
  readTable(table: Table, fieldName: string): Table|null {
    const field = this.getField(fieldName, table.typeIndex);
    const fieldType = field.type();
    const parentIsStruct = this.getType(table.typeIndex).isStruct();
    if (fieldType.baseType() !== BaseType.Obj) {
      throw new Error('Field ' + fieldName + ' is not an object type.');
    }

    if (parentIsStruct) {
      return new Table(
          table.bb, fieldType.index(), table.offset + field.offset());
    }

    const offsetToOffset =
        table.offset + table.bb.__offset(table.offset, field.offset());
    if (offsetToOffset === table.offset) {
      return null;
    }

    const elementIsStruct = this.getType(fieldType.index()).isStruct();

    const objectStart =
        elementIsStruct ? offsetToOffset : table.bb.__indirect(offsetToOffset);
    return new Table(table.bb, fieldType.index(), objectStart);
  }
  // Reads a vector of scalars (like readScalar, may return a vector of Long's
  // instead). Also, will return null if the vector is not set.
  readVectorOfScalars(table: Table, fieldName: string): number[]|null {
    const field = this.getField(fieldName, table.typeIndex);
    const fieldType = field.type();
    if (fieldType.baseType() !== BaseType.Vector) {
      throw new Error('Field ' + fieldName + ' is not an vector.');
    }
    if (!isScalar(fieldType.element())) {
      throw new Error('Field ' + fieldName + ' is not an vector of scalars.');
    }

    const offsetToOffset =
        table.offset + table.bb.__offset(table.offset, field.offset());
    if (offsetToOffset === table.offset) {
      return null;
    }
    const numElements = table.bb.__vector_len(offsetToOffset);
    const result = [];
    const baseOffset = table.bb.__vector(offsetToOffset);
    const scalarSize = typeSize(fieldType.element());
    for (let ii = 0; ii < numElements; ++ii) {
      result.push(
          table.readScalar(fieldType.element(), baseOffset + scalarSize * ii));
    }
    return result;
  }
  // Reads a vector of tables. Returns null if vector is not set.
  readVectorOfTables(table: Table, fieldName: string) {
    const field = this.getField(fieldName, table.typeIndex);
    const fieldType = field.type();
    if (fieldType.baseType() !== BaseType.Vector) {
      throw new Error('Field ' + fieldName + ' is not an vector.');
    }
    if (fieldType.element() !== BaseType.Obj) {
      throw new Error('Field ' + fieldName + ' is not an vector of objects.');
    }

    const offsetToOffset =
        table.offset + table.bb.__offset(table.offset, field.offset());
    if (offsetToOffset === table.offset) {
      return null;
    }
    const numElements = table.bb.__vector_len(offsetToOffset);
    const result = [];
    const baseOffset = table.bb.__vector(offsetToOffset);
    const elementSchema = this.getType(fieldType.index());
    const elementIsStruct = elementSchema.isStruct();
    const elementSize = elementIsStruct ? elementSchema.bytesize() :
                                          typeSize(fieldType.element());
    for (let ii = 0; ii < numElements; ++ii) {
      const elementOffset = baseOffset + elementSize * ii;
      result.push(new Table(
          table.bb, fieldType.index(),
          elementIsStruct ? elementOffset :
                            table.bb.__indirect(elementOffset)));
    }
    return result;
  }
  // Reads a vector of strings. Returns null if not set.
  readVectorOfStrings(table: Table, fieldName: string): string[]|null {
    const field = this.getField(fieldName, table.typeIndex);
    const fieldType = field.type();
    if (fieldType.baseType() !== BaseType.Vector) {
      throw new Error('Field ' + fieldName + ' is not an vector.');
    }
    if (fieldType.element() !== BaseType.String) {
      throw new Error('Field ' + fieldName + ' is not an vector of strings.');
    }

    const offsetToOffset =
        table.offset + table.bb.__offset(table.offset, field.offset());
    if (offsetToOffset === table.offset) {
      return null;
    }
    const numElements = table.bb.__vector_len(offsetToOffset);
    const result = [];
    const baseOffset = table.bb.__vector(offsetToOffset);
    const offsetSize = typeSize(fieldType.element());
    for (let ii = 0; ii < numElements; ++ii) {
      result.push(table.bb.__string(baseOffset + offsetSize * ii));
    }
    return result;
  }
}