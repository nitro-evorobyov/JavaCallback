/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.10
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.nitro.cloud;

public class Task {
  private transient long swigCPtr;
  protected transient boolean swigCMemOwn;

  protected Task(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Task obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        nitro_cloudJNI.delete_Task(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public void Run() {
    nitro_cloudJNI.Task_Run(swigCPtr, this);
  }

}