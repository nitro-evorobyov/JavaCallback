package com.nitro.pdf_box.task;

import com.nitro.cloud.TaskCallbackCommnad;

/**
 * Created by evorobyov on 10.08.2016.
 */
public class TaskThrowFromJava  extends TaskSimple {
    private int taskThrowStep;

    public  TaskThrowFromJava(int throwStep, int sleepCount, int failedStep) {
        super(sleepCount, failedStep);
        taskThrowStep = throwStep;
    }

    public  TaskThrowFromJava(int throwStep, int sleepCount) {
        super(sleepCount);
        taskThrowStep = throwStep;
    }



    public TaskCallbackCommnad OnProgress(int progress, int upperBound, String someMessage) {
        if (progress == taskThrowStep)
        {
            throw new RuntimeException("RuntimeException!!! Hello form java.");
        }
        return super.OnProgress(progress, upperBound, someMessage);
    }
}
