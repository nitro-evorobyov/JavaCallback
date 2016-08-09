package com.nitro.pdf_box;

import com.nitro.cloud.TaskSwig;
import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Native;

import java.util.Random;
import java.util.Vector;


class TaskJna implements Callback {
    Vector<TaskJna> taskRegistrator;

    public void callback (String param1, String param2) {
        System.out.println(param1+" "+param2);

        if (param2.contains("Finished."))
        {
            synchronized (taskRegistrator) {
                taskRegistrator.remove(this);
            }
        }
    }

    TaskJna(Vector<TaskJna> taskCollection)
    {
        taskRegistrator = taskCollection;

        synchronized (taskRegistrator) {
            taskRegistrator.add(this);
        }
    }
}

interface TaskJnaLibrary extends Library {
    void RunSimple(int stepsCount, TaskJna callback);
}

public class Main {


    public static void main(String[] args) {
        SwigRun();
        JnaRun();
    }

    private static void JnaRun() {
        System.out.println("************************************************");
        System.out.println("*******************JNA - test*******************");
        System.out.println("************************************************");

        TaskJnaLibrary lib = (TaskJnaLibrary) Native.loadLibrary("nitro_cloud", TaskJnaLibrary.class);
        Random rand = new Random();

        Vector<TaskJna> taskCollection = new Vector<TaskJna>();

        for (int threadIndex = 0; threadIndex < 10; ++threadIndex) {
            lib.RunSimple(rand.nextInt(4) + 5, new TaskJna(taskCollection));
        }

        while (true) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            synchronized (taskCollection) {
                if (taskCollection.isEmpty()) {
                    break;
                }
            }
        }
    }

    private static void SwigRun() {
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
                if (rand.nextInt(10) == 7) {
                    failedStep = rand.nextInt(20);
                }

                JavaTask task = new JavaTask(upperBound, failedStep);
                task.Register(taskCollection);
                task.Run();
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
