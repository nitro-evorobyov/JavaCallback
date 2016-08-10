package com.nitro.pdf_box.task;

import com.nitro.cloud.TaskCallbackCommnad;

/**
 * Created by evorobyov on 29.07.2016.
 */
public class TaskCancelFromJava extends TaskSimple {

    private int taskCancelStep;

    public TaskCancelFromJava(int cancelStep, int sleepCount, int failedStep) {
        super(sleepCount, failedStep);
        taskCancelStep = cancelStep;
    }

    public TaskCancelFromJava(int cancelStep, int sleepCount) {
        super(sleepCount);
        taskCancelStep = cancelStep;
    }



    public TaskCallbackCommnad OnProgress(int progress, int upperBound, String someMessage) {
        if (progress == taskCancelStep)
        {
            return TaskCallbackCommnad.TaskCancel;
        }
        return super.OnProgress(progress, upperBound, someMessage);
    }


}
