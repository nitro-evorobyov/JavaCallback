package com.nitro.pdf_box;

import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Native;

import java.util.Random;
import java.util.Vector;

/**
 * Created by evorobyov on 10.08.2016.
 */


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

public class JnaMain {


    public static void Run() {
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

}
