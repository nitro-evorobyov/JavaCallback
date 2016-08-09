package com.nitro.pdf_box;

import com.nitro.cloud.TaskCallbackCommnad;
import com.nitro.cloud.TaskResult;
import com.nitro.cloud.TaskSwig;

import java.io.PrintStream;
import java.util.Vector;

/**
 * Created by evorobyov on 28.07.2016.
 */
public class JavaTask extends TaskSwig {

    private Vector<TaskSwig> registerTaskCollection = null;

    public JavaTask (int sleepCount, int failedStep) {
        super(sleepCount, failedStep);

    }

    public JavaTask (int sleepCount)
    {
        super(sleepCount);
    }

    public void Register(Vector<TaskSwig> taskCollection)
    {
        registerTaskCollection  = taskCollection;

        if(registerTaskCollection  != null)
        {
            synchronized(registerTaskCollection){

                if(!registerTaskCollection.contains(this)) {
                    registerTaskCollection.add(this);
                }
            }
        }
    }

    public TaskCallbackCommnad OnStarted(String someMessage) {
        PrintStream logger = System.out;
        synchronized(logger) {
            logger.println(someMessage);
        }
        return TaskCallbackCommnad.TaskContinue;
    }

    public TaskCallbackCommnad OnProgress(int progress, int upperBound, String someMessage) {
        PrintStream logger = System.out;
        synchronized(logger) {
            logger.print(someMessage);
            logger.print(" ");
            logger.print(progress);
            logger.print("/");
            logger.print(upperBound);
            logger.println(" steps.");
        }
        return TaskCallbackCommnad.TaskContinue;
    }

    public void OnFinished(TaskResult taskResult, String someMessage) {
        if(registerTaskCollection!= null) {
            synchronized(registerTaskCollection) {
                registerTaskCollection.remove(this);
            }
        }

        PrintStream logger = System.out;
        synchronized(logger) {
            logger.println(someMessage);
        }
    }
}
