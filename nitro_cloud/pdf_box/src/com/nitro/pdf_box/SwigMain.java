package com.nitro.pdf_box;

import com.nitro.cloud.TaskSwig;
import com.nitro.pdf_box.task.TaskCancelFromJava;
import com.nitro.pdf_box.task.TaskSimple;
import com.nitro.pdf_box.task.TaskThrowFromJava;

import java.util.Random;
import java.util.Vector;

/**
 * Created by evorobyov on 10.08.2016.
 */
public class SwigMain {

    enum  SwigTaskType
    {
        Random,
        Simple,
        JavaCancel,
        JavaThrow,
    }

    public static void Run(SwigTaskType taskType) {
        Vector<TaskSwig> taskCollection = new Vector<TaskSwig>();

        System.out.println("************************************************");
        System.out.println("*******************SWIG- test*******************");
        System.out.println("************************************************");

        try {
            System.loadLibrary("nitro_cloud");

            Random rand = new Random();

            // nextInt is normally exclusive of the top value,
            // so add 1 to make it inclusive

            for (int i = 0; i < 10; ++i) {
                int upperBound = rand.nextInt(10) + 10;

                int failedStep = -1;
                if (rand.nextInt(10) > 7) {
                    failedStep = rand.nextInt(20);
                }

                SwigTaskType currentTaskType = taskType;

                if(currentTaskType == SwigTaskType.Random) {
                    currentTaskType = SwigTaskType.values()[rand.nextInt(2)];
                }

                switch (currentTaskType) {
                    case Simple:
                        TaskSimple taskSimple = new TaskSimple(upperBound, failedStep);
                        taskSimple.Register(taskCollection);
                        taskSimple.Run();
                        break;
                    case JavaCancel:
                        TaskCancelFromJava taskCancle = new TaskCancelFromJava(rand.nextInt(upperBound), upperBound);
                        taskCancle.Register(taskCollection);
                        taskCancle.Run();
                        break;
                    case JavaThrow:
                        TaskThrowFromJava taskJavaThrow = new TaskThrowFromJava(rand.nextInt(upperBound), upperBound);
                        taskJavaThrow.Register(taskCollection);
                        taskJavaThrow.Run();
                        break;
                }

            }

            while (true) {
                Thread.sleep(100);

                synchronized (taskCollection) {
                    if (taskCollection.isEmpty()) {
                        break;
                    }
                }
            }
        } catch (Exception ex) {

        }
    }
}
