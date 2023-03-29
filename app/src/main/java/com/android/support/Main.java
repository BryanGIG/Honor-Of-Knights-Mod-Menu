package com.android.support;

import android.app.Activity;
import android.content.Context;
import android.graphics.PixelFormat;
import android.view.View;
import android.view.WindowManager;

public class Main {

    public static void StartWithoutPermission(Context activity) {
        if (activity instanceof Activity) {
            System.loadLibrary("native");
            JNIBridge.onCreate(activity);
            GUISurface surfaceGUI = new GUISurface(activity);
            surfaceGUI.setZOrderOnTop(true);
            surfaceGUI.getHolder().setFormat(PixelFormat.TRANSPARENT);
            WindowManager.LayoutParams params = new WindowManager.LayoutParams();
            params.width = WindowManager.LayoutParams.MATCH_PARENT;
            params.height = WindowManager.LayoutParams.MATCH_PARENT;
            ((Activity) activity).addContentView(surfaceGUI, params);
            surfaceGUI.setVisibility(View.VISIBLE);
        }
    }
}
