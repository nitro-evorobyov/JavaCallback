package com.nitro.pdf_box.task;

import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;

/**
 * Created by evorobyov on 10.08.2016.
 */
public class MessageCollector {

    PrintWriter messageWriter = null;

    public MessageCollector(String path)
    {

        try {
            messageWriter = new PrintWriter (path, "UTF-8");
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

    public void Collect(String message)
    {
        messageWriter.println(message);
    }

    public void Finished()
    {
        messageWriter.flush();
        messageWriter.close();
    }
}
