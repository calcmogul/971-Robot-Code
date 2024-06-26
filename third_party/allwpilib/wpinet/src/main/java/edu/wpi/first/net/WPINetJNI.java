// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

package edu.wpi.first.net;

import edu.wpi.first.util.RuntimeLoader;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;

/** WPINet JNI. */
public class WPINetJNI {
  static boolean libraryLoaded = false;
  static RuntimeLoader<WPINetJNI> loader = null;

  /** Sets whether JNI should be loaded in the static block. */
  public static class Helper {
    private static AtomicBoolean extractOnStaticLoad = new AtomicBoolean(true);

    /**
     * Returns true if the JNI should be loaded in the static block.
     *
     * @return True if the JNI should be loaded in the static block.
     */
    public static boolean getExtractOnStaticLoad() {
      return extractOnStaticLoad.get();
    }

    /**
     * Sets whether the JNI should be loaded in the static block.
     *
     * @param load Whether the JNI should be loaded in the static block.
     */
    public static void setExtractOnStaticLoad(boolean load) {
      extractOnStaticLoad.set(load);
    }

    /** Utility class. */
    private Helper() {}
  }

  static {
    if (Helper.getExtractOnStaticLoad()) {
      try {
        loader =
            new RuntimeLoader<>(
                "wpinetjni", RuntimeLoader.getDefaultExtractionRoot(), WPINetJNI.class);
        loader.loadLibrary();
      } catch (IOException ex) {
        ex.printStackTrace();
        System.exit(1);
      }
      libraryLoaded = true;
    }
  }

  /**
   * Force load the library.
   *
   * @throws IOException if the library failed to load
   */
  public static synchronized void forceLoad() throws IOException {
    if (libraryLoaded) {
      return;
    }
    loader =
        new RuntimeLoader<>("wpinetjni", RuntimeLoader.getDefaultExtractionRoot(), WPINetJNI.class);
    loader.loadLibrary();
    libraryLoaded = true;
  }

  /**
   * Forward a local TCP port to a remote host and port. Note that local ports less than 1024 won't
   * work as a normal user.
   *
   * @param port local port number
   * @param remoteHost remote IP address / DNS name
   * @param remotePort remote port number
   */
  public static native void addPortForwarder(int port, String remoteHost, int remotePort);

  /**
   * Stop TCP forwarding on a port.
   *
   * @param port local port number
   */
  public static native void removePortForwarder(int port);

  public static native int createMulticastServiceAnnouncer(
      String serviceName, String serviceType, int port, String[] keys, String[] values);

  public static native void freeMulticastServiceAnnouncer(int handle);

  public static native void startMulticastServiceAnnouncer(int handle);

  public static native void stopMulticastServiceAnnouncer(int handle);

  public static native boolean getMulticastServiceAnnouncerHasImplementation(int handle);

  public static native int createMulticastServiceResolver(String serviceType);

  public static native void freeMulticastServiceResolver(int handle);

  public static native void startMulticastServiceResolver(int handle);

  public static native void stopMulticastServiceResolver(int handle);

  public static native boolean getMulticastServiceResolverHasImplementation(int handle);

  public static native int getMulticastServiceResolverEventHandle(int handle);

  public static native ServiceData[] getMulticastServiceResolverData(int handle);

  /** Utility class. */
  private WPINetJNI() {}
}
