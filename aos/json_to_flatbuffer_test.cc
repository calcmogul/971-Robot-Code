#include "aos/json_to_flatbuffer.h"

#include "flatbuffers/minireflect.h"
#include "gtest/gtest.h"

#include "aos/flatbuffer_merge.h"
#include "aos/json_to_flatbuffer_generated.h"
#include "aos/testing/path.h"

namespace aos::testing {

class JsonToFlatbufferTest : public ::testing::Test {
 public:
  enum class TestReflection { kYes, kNo };

  JsonToFlatbufferTest() {}

  FlatbufferVector<reflection::Schema> Schema() {
    return FileToFlatbuffer<reflection::Schema>(
        ArtifactPath("aos/json_to_flatbuffer.bfbs"));
  }

  // JsonAndBack tests using both the reflection::Schema* as well as the
  // minireflect tables for both parsing and outputting JSON. However, there are
  // currently minor discrepencies between how the JSON output works for the two
  // modes, so some tests must manually disable testing of the
  // FlatbufferToJson() overload that takes a reflection::Schema*.
  bool JsonAndBack(const char *str, TestReflection test_reflection_to_json =
                                        TestReflection::kYes) {
    return JsonAndBack(str, str, test_reflection_to_json);
  }

  bool JsonAndBack(
      const char *in, const char *out,
      TestReflection test_reflection_to_json = TestReflection::kYes) {
    FlatbufferDetachedBuffer<Configuration> fb_typetable =
        JsonToFlatbuffer<Configuration>(in);
    FlatbufferDetachedBuffer<Configuration> fb_reflection =
        JsonToFlatbuffer(in, FlatbufferType(&Schema().message()));

    if (fb_typetable.span().size() == 0) {
      return false;
    }
    if (fb_reflection.span().size() == 0) {
      return false;
    }

    const ::std::string back_typetable = FlatbufferToJson(fb_typetable);
    const ::std::string back_reflection = FlatbufferToJson(fb_reflection);
    const ::std::string back_reflection_reflection =
        FlatbufferToJson(&Schema().message(), fb_reflection.span().data());

    printf("Back to table via TypeTable and to string via TypeTable: %s\n",
           back_typetable.c_str());
    printf("Back to table via reflection and to string via TypeTable: %s\n",
           back_reflection.c_str());
    if (test_reflection_to_json == TestReflection::kYes) {
      printf("Back to table via reflection and to string via reflection: %s\n",
             back_reflection_reflection.c_str());
    }

    const bool as_expected =
        back_typetable == out && back_reflection == out &&
        ((test_reflection_to_json == TestReflection::kNo) ||
         (back_reflection_reflection == out));
    if (!as_expected) {
      printf("But expected: %s\n", out);
    }
    return as_expected;
  }
};

// Tests that the various escapes work as expected.
TEST_F(JsonToFlatbufferTest, ValidEscapes) {
  EXPECT_TRUE(
      JsonAndBack("{ \"foo_string\": \"a\\\"b\\/c\\bd\\fc\\nd\\re\\tf\" }",
                  "{ \"foo_string\": \"a\\\"b/c\\bd\\fc\\nd\\re\\tf\" }"));
}

// Test the easy ones.  Test every type, single, no nesting.
TEST_F(JsonToFlatbufferTest, Basic) {
  EXPECT_TRUE(JsonAndBack("{ \"foo_bool\": true }"));

  EXPECT_TRUE(JsonAndBack("{ \"foo_byte\": 5 }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_ubyte\": 5 }"));

  EXPECT_TRUE(JsonAndBack("{ \"foo_short\": 5 }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_ushort\": 5 }"));

  EXPECT_TRUE(JsonAndBack("{ \"foo_int\": 5 }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_uint\": 5 }"));

  EXPECT_TRUE(JsonAndBack("{ \"foo_long\": 5 }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_ulong\": 5 }"));

  EXPECT_TRUE(JsonAndBack("{ \"foo_float\": 5 }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_float\": 50 }"));
  // Test that we can distinguish between floats that vary by a single bit.
  EXPECT_TRUE(JsonAndBack("{ \"foo_float\": 1.1 }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_float\": 1.0999999 }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_double\": 5 }"));
  // Check that we handle/distinguish between doubles that vary by a single bit.
  EXPECT_TRUE(JsonAndBack("{ \"foo_double\": 1.561154546713 }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_double\": 1.56115454671299 }"));

  EXPECT_TRUE(JsonAndBack("{ \"foo_enum\": \"None\" }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_enum\": \"UType\" }"));

  EXPECT_TRUE(JsonAndBack("{ \"foo_enum_default\": \"None\" }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_enum_default\": \"UType\" }"));

  EXPECT_TRUE(JsonAndBack("{ \"foo_string\": \"baz\" }"));

  EXPECT_TRUE(JsonAndBack("{ \"foo_enum_nonconsecutive\": \"Zero\" }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_enum_nonconsecutive\": \"Big\" }"));
}

TEST_F(JsonToFlatbufferTest, Structs) {
  EXPECT_TRUE(
      JsonAndBack("{ \"foo_struct\": { \"foo_byte\": 1, \"nested_struct\": { "
                  "\"foo_byte\": 2 } } }"));
  EXPECT_TRUE(JsonAndBack(
      "{ \"foo_struct_scalars\": { \"foo_float\": 1.234, \"foo_double\": "
      "4.567, \"foo_int32\": -971, \"foo_uint32\": 4294967294, "
      "\"foo_int64\": -1030, \"foo_uint64\": 18446744073709551614 } }",
      TestReflection::kNo));
  // Confirm that we parse integers into floating point fields correctly.
  EXPECT_TRUE(JsonAndBack(
      "{ \"foo_struct_scalars\": { \"foo_float\": 1, \"foo_double\": "
      "2, \"foo_int32\": 3, \"foo_uint32\": 4, \"foo_int64\": "
      "5, \"foo_uint64\": 6 } }",
      TestReflection::kNo));
  EXPECT_TRUE(JsonAndBack(
      "{ \"vector_foo_struct_scalars\": [ { \"foo_float\": 1.234, "
      "\"foo_double\": 4.567, \"foo_int32\": -971, "
      "\"foo_uint32\": 4294967294, \"foo_int64\": -1030, \"foo_uint64\": "
      "18446744073709551614 }, { \"foo_float\": 2, \"foo_double\": "
      "4.1, \"foo_int32\": 10, \"foo_uint32\": 13, "
      "\"foo_int64\": 15, \"foo_uint64\": 18 } ] }",
      TestReflection::kNo));
  EXPECT_TRUE(
      JsonAndBack("{ \"foo_struct_enum\": { \"foo_enum\": \"UByte\" } }"));
  EXPECT_TRUE(
      JsonAndBack("{ \"vector_foo_struct\": [ { \"foo_byte\": 1, "
                  "\"nested_struct\": { \"foo_byte\": 2 } } ] }"));
  EXPECT_TRUE(JsonAndBack(
      "{ \"vector_foo_struct\": [ { \"foo_byte\": 1, \"nested_struct\": { "
      "\"foo_byte\": 2 } }, { \"foo_byte\": 3, \"nested_struct\": { "
      "\"foo_byte\": 4 } }, { \"foo_byte\": 5, \"nested_struct\": { "
      "\"foo_byte\": 6 } } ] }"));
}

// Confirm that we correctly die when input JSON is missing fields inside of a
// struct.
TEST_F(JsonToFlatbufferTest, StructMissingField) {
  ::testing::internal::CaptureStderr();
  EXPECT_FALSE(
      JsonAndBack("{ \"foo_struct\": { \"nested_struct\": { "
                  "\"foo_byte\": 2 } } }"));
  EXPECT_FALSE(JsonAndBack(
      "{ \"foo_struct\": { \"foo_byte\": 1, \"nested_struct\": {  } } }"));
  EXPECT_FALSE(JsonAndBack("{ \"foo_struct\": { \"foo_byte\": 1 } }"));
  std::string output = ::testing::internal::GetCapturedStderr();
  EXPECT_EQ(
      R"output(All fields must be specified for struct types (field foo_byte missing).
All fields must be specified for struct types (field foo_byte missing).
All fields must be specified for struct types (field foo_byte missing).
All fields must be specified for struct types (field foo_byte missing).
All fields must be specified for struct types (field nested_struct missing).
All fields must be specified for struct types (field nested_struct missing).
)output",
      output);
}

// Tests that Inf is handled correctly
TEST_F(JsonToFlatbufferTest, Inf) {
  EXPECT_TRUE(JsonAndBack("{ \"foo_float\": inf }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_float\": -inf }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_double\": inf }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_double\": -inf }"));
}

// Tests that NaN is handled correctly
TEST_F(JsonToFlatbufferTest, Nan) {
  EXPECT_TRUE(JsonAndBack("{ \"foo_float\": nan }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_float\": -nan }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_double\": nan }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_double\": -nan }"));
}

// Tests that unicode is handled correctly
TEST_F(JsonToFlatbufferTest, Unicode) {
  // The reflection-based FlatbufferToJson outputs actual unicode rather than
  // escaped code-points.
  EXPECT_TRUE(
      JsonAndBack("{ \"foo_string\": \"\\uF672\" }", TestReflection::kNo));
  EXPECT_TRUE(
      JsonAndBack("{ \"foo_string\": \"\\uEFEF\" }", TestReflection::kNo));
  EXPECT_TRUE(JsonAndBack("{ \"foo_string\": \"helloworld\\uD83E\\uDE94\" }",
                          TestReflection::kNo));
  EXPECT_TRUE(JsonAndBack("{ \"foo_string\": \"\\uD83C\\uDF32\" }",
                          TestReflection::kNo));
  EXPECT_FALSE(
      JsonAndBack("{ \"foo_string\": \"\\uP890\" }", TestReflection::kNo));
  EXPECT_FALSE(
      JsonAndBack("{ \"foo_string\": \"\\u!FA8\" }", TestReflection::kNo));
  EXPECT_FALSE(
      JsonAndBack("{ \"foo_string\": \"\\uF89\" }", TestReflection::kNo));
  EXPECT_FALSE(
      JsonAndBack("{ \"foo_string\": \"\\uD83C\" }", TestReflection::kNo));
}

// Tests that we can handle decimal points.
TEST_F(JsonToFlatbufferTest, DecimalPoint) {
  EXPECT_TRUE(JsonAndBack("{ \"foo_float\": 5.099999 }"));
  EXPECT_TRUE(JsonAndBack("{ \"foo_double\": 5.099999999999 }"));
}

// Test what happens if you pass a field name that we don't know.
TEST_F(JsonToFlatbufferTest, InvalidFieldName) {
  EXPECT_FALSE(JsonAndBack("{ \"foo\": 5 }"));
}

// Tests that an invalid enum type is handled correctly.
TEST_F(JsonToFlatbufferTest, InvalidEnumName) {
  EXPECT_FALSE(JsonAndBack("{ \"foo_enum\": \"5ype\" }"));

  EXPECT_FALSE(JsonAndBack("{ \"foo_enum_default\": \"7ype\" }"));

  EXPECT_FALSE(JsonAndBack("{ \"foo_enum_nonconsecutive\": \"Nope\" }"));

  EXPECT_FALSE(
      JsonAndBack("{ \"foo_enum_nonconsecutive_default\": \"Nope\" }"));
}

// Test that adding a duplicate field results in an error.
TEST_F(JsonToFlatbufferTest, DuplicateField) {
  EXPECT_FALSE(
      JsonAndBack("{ \"foo_int\": 5, \"foo_int\": 7 }", "{ \"foo_int\": 7 }"));
}

// Test that various syntax errors are caught correctly
TEST_F(JsonToFlatbufferTest, InvalidSyntax) {
  EXPECT_FALSE(JsonAndBack("{ \"foo_int\": 5"));
  EXPECT_FALSE(JsonAndBack("{ \"foo_int\": 5 "));
  EXPECT_FALSE(JsonAndBack("{ \"foo_string\": \""));
  EXPECT_FALSE(JsonAndBack("{ \"foo_int\": 5 } }"));

  EXPECT_FALSE(JsonAndBack("{ foo_int: 5 }"));

  EXPECT_FALSE(JsonAndBack("{ \"foo_int\": 5, }", "{ \"foo_int\": 5 }"));

  EXPECT_FALSE(
      JsonAndBack("{ \"apps\":\n[\n{\n\"name\": \"woot\"\n},\n{\n\"name\": "
                  "\"wow\"\n} ,\n]\n}"));

  EXPECT_FALSE(JsonAndBack(
      "{ \"apps\": [ { \"name\": \"woot\" }, { \"name\": \"wow\" } ] , }"));

  EXPECT_FALSE(
      JsonAndBack("{ \"vector_foo_string\": [ \"bar\", \"baz\" ] , }"));

  EXPECT_FALSE(
      JsonAndBack("{ \"single_application\": { \"name\": \"woot\" } , }"));
}

// Test arrays of simple types.
TEST_F(JsonToFlatbufferTest, Array) {
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_byte\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_byte\": [  ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_ubyte\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_ubyte\": [  ] }"));

  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_short\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_short\": [  ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_ushort\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_ushort\": [  ] }"));

  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_int\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_int\": [  ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_uint\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_uint\": [  ] }"));

  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_long\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_long\": [  ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_ulong\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_ulong\": [  ] }"));

  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_float\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_float\": [  ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_double\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_double\": [  ] }"));

  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_float\": [ 9.0, 7.0, 1.0 ] }",
                          "{ \"vector_foo_float\": [ 9, 7, 1 ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_double\": [ 9.0, 7.0, 1.0 ] }",
                          "{ \"vector_foo_double\": [ 9, 7, 1 ] }"));

  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_string\": [ \"bar\", \"baz\" ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_string\": [  ] }"));
  EXPECT_TRUE(JsonAndBack(
      "{ \"vector_foo_enum\": [ \"None\", \"UType\", \"Bool\" ] }"));
  EXPECT_TRUE(JsonAndBack("{ \"vector_foo_enum\": [  ] }"));
}

// Test nested messages, and arrays of nested messages.
TEST_F(JsonToFlatbufferTest, NestedTable) {
  EXPECT_TRUE(
      JsonAndBack("{ \"single_application\": { \"name\": \"woot\" } }"));

  EXPECT_TRUE(JsonAndBack("{ \"single_application\": {  } }"));

  EXPECT_TRUE(JsonAndBack(
      "{ \"apps\": [ { \"name\": \"woot\" }, { \"name\": \"wow\" } ] }"));

  EXPECT_TRUE(JsonAndBack("{ \"apps\": [ {  }, {  } ] }"));
}

// Test mixing up whether a field is an object or a vector.
TEST_F(JsonToFlatbufferTest, IncorrectVectorOfTables) {
  EXPECT_FALSE(
      JsonAndBack("{ \"single_application\": [ {\"name\": \"woot\"} ] }"));
  EXPECT_FALSE(JsonAndBack("{ \"apps\": { \"name\": \"woot\" } }"));
}

// Test that we can parse an empty message.
TEST_F(JsonToFlatbufferTest, EmptyMessage) {
  // Empty message works.
  EXPECT_TRUE(JsonAndBack("{  }"));
}

// Tests that C style comments get stripped.
TEST_F(JsonToFlatbufferTest, CStyleComments) {
  EXPECT_TRUE(JsonAndBack(R"({
  /* foo */
  "vector_foo_double": [ 9, 7, 1 ] /* foo */
} /* foo */)",
                          "{ \"vector_foo_double\": [ 9, 7, 1 ] }"));
}

// Tests that C++ style comments get stripped.
TEST_F(JsonToFlatbufferTest, CppStyleComments) {
  EXPECT_TRUE(JsonAndBack(R"({
  // foo
  "vector_foo_double": [ 9, 7, 1 ] // foo
} // foo)",
                          "{ \"vector_foo_double\": [ 9, 7, 1 ] }"));

  // Test empty comment on its own line doesn't remove the next line.
  EXPECT_TRUE(JsonAndBack(R"({
  //
  "vector_foo_double": [ 9, 7, 1 ], // foo
  "vector_foo_float": [ 3, 1, 4 ]
} // foo)",
                          "{ \"vector_foo_float\": [ 3, 1, 4 ], "
                          "\"vector_foo_double\": [ 9, 7, 1 ] }",
                          TestReflection::kNo));

  // Test empty comment at end of line doesn't remove the next line.
  EXPECT_TRUE(JsonAndBack(R"({
  // foo
  "vector_foo_double": [ 2, 7, 1 ], //
  "vector_foo_float": [ 3, 1, 4 ]
} // foo)",
                          "{ \"vector_foo_float\": [ 3, 1, 4 ], "
                          "\"vector_foo_double\": [ 2, 7, 1 ] }",
                          TestReflection::kNo));

  // Test empty comment at end of document doesn't cause error.
  EXPECT_TRUE(JsonAndBack(R"({
  // foo
  "vector_foo_double": [ 5, 6, 7 ], // foo
  "vector_foo_float": [ 7, 8, 9 ]
} //)",
                          "{ \"vector_foo_float\": [ 7, 8, 9 ], "
                          "\"vector_foo_double\": [ 5, 6, 7 ] }",
                          TestReflection::kNo));
}

// Tests that mixed style comments get stripped.
TEST_F(JsonToFlatbufferTest, MixedStyleComments) {
  // Weird comments do not throw us off.
  EXPECT_TRUE(JsonAndBack(R"({
  // foo /* foo */
  "vector_foo_double": [ 9, 7, 1 ] /* // foo */
}
// foo
/* foo */)",
                          "{ \"vector_foo_double\": [ 9, 7, 1 ] }",
                          TestReflection::kYes));
}

// Tests that multiple arrays get properly handled.
TEST_F(JsonToFlatbufferTest, MultipleArrays) {
  EXPECT_TRUE(
      JsonAndBack("{ \"vector_foo_float\": [ 9, 7, 1 ], \"vector_foo_double\": "
                  "[ 9, 7, 1 ] }",
                  TestReflection::kNo));
}

// Tests that multiple arrays get properly handled.
TEST_F(JsonToFlatbufferTest, NestedArrays) {
  EXPECT_TRUE(
      JsonAndBack("{ \"vov\": { \"v\": [ { \"str\": [ \"a\", \"b\" ] }, { "
                  "\"str\": [ \"c\", \"d\" ] } ] } }"));
}

// TODO(austin): Missmatched values.
//
// TODO(austin): unions?

TEST_F(JsonToFlatbufferTest, TrimmedVector) {
  std::string json_short = "{ \"vector_foo_int\": [ 0";
  for (int i = 1; i < 100; ++i) {
    json_short += ", ";
    json_short += std::to_string(i);
  }
  std::string json_long = json_short;
  json_short += " ] }";
  json_long += ", 101 ] }";

  const FlatbufferDetachedBuffer<Configuration> fb_short_typetable(
      JsonToFlatbuffer<Configuration>(json_short));
  ASSERT_GT(fb_short_typetable.span().size(), 0);
  const FlatbufferDetachedBuffer<Configuration> fb_long_typetable(
      JsonToFlatbuffer<Configuration>(json_long));
  ASSERT_GT(fb_long_typetable.span().size(), 0);
  const FlatbufferDetachedBuffer<Configuration> fb_short_reflection(
      JsonToFlatbuffer(json_short, FlatbufferType(&Schema().message())));
  ASSERT_GT(fb_short_reflection.span().size(), 0);
  const FlatbufferDetachedBuffer<Configuration> fb_long_reflection(
      JsonToFlatbuffer(json_long, FlatbufferType(&Schema().message())));
  ASSERT_GT(fb_long_reflection.span().size(), 0);

  const std::string back_json_short_typetable = FlatbufferToJson<Configuration>(
      fb_short_typetable, {.multi_line = false, .max_vector_size = 100});
  const std::string back_json_long_typetable = FlatbufferToJson<Configuration>(
      fb_long_typetable, {.multi_line = false, .max_vector_size = 100});
  const std::string back_json_short_reflection =
      FlatbufferToJson<Configuration>(
          fb_short_reflection, {.multi_line = false, .max_vector_size = 100});
  const std::string back_json_long_reflection = FlatbufferToJson<Configuration>(
      fb_long_reflection, {.multi_line = false, .max_vector_size = 100});

  EXPECT_EQ(json_short, back_json_short_typetable);
  EXPECT_EQ(json_short, back_json_short_reflection);
  EXPECT_EQ("{ \"vector_foo_int\": [ \"... 101 elements ...\" ] }",
            back_json_long_typetable);
  EXPECT_EQ("{ \"vector_foo_int\": [ \"... 101 elements ...\" ] }",
            back_json_long_reflection);
}

// Tests that a nullptr buffer prints nullptr.
TEST_F(JsonToFlatbufferTest, NullptrData) {
  EXPECT_EQ("null", TableFlatbufferToJson((const flatbuffers::Table *)(nullptr),
                                          ConfigurationTypeTable()));
}

TEST_F(JsonToFlatbufferTest, SpacedData) {
  EXPECT_TRUE(CompareFlatBuffer(
      FlatbufferDetachedBuffer<VectorOfStrings>(
          JsonToFlatbuffer<VectorOfStrings>(R"json({
	"str": [
		"f o o",
		"b a r",
		"foo bar",
		"bar foo"
	]
})json")),
      JsonFileToFlatbuffer<VectorOfStrings>(
          ArtifactPath("aos/json_to_flatbuffer_test_spaces.json"))));
}

}  // namespace aos::testing
