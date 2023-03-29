package com.android.support;

import static com.android.support.JNIBridge.injectEvent;

import android.content.Context;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

public class GUISurface extends SurfaceView implements SurfaceHolder.Callback {

    public GUISurface(Context ctx) {
        super(ctx);
        getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        JNIBridge.surfaceCreate(getHolder().getSurface(), getWidth(), getHeight());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        JNIBridge.surfaceChanged(width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        JNIBridge.surfaceDestroyed();
    }
}