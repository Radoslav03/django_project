from django.shortcuts import render
from django.http import HttpResponse
from django.http import StreamingHttpResponse
from django.views import generic
from django.http import JsonResponse
import picamera
import io
from threading import Condition
from time import sleep

import sys
import json
from sps30 import SPS30

import serial


class StreamingOutput(object):
    def __init__(self):
        self.frame = None
        self.buffer = io.BytesIO()
        self.condition = Condition()

    def write(self, buf):
        if buf.startswith(b'\xff\xd8'):
            self.buffer.truncate()
            with self.condition:
                self.frame = self.buffer.getvalue()
                self.condition.notify_all()
            self.buffer.seek(0)
        return self.buffer.write(buf)

def stream(request):

        def cam():
                with picamera.PiCamera(resolution='1920x1080', framerate=30) as camera:
                        output = StreamingOutput()
                        camera.start_recording(output, format='mjpeg')
                        camera.rotation = 180
                        
                        sleep(1)
                        try:
                                while True:
                                        with output.condition:
                                                output.condition.wait()
                                                frame = output.frame
                                        yield(b'--frame\r\n'
                                                b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n\r\n')

                        finally:
                                camera.stop_recording()

        try:
                return StreamingHttpResponse(cam(), content_type="multipart/x-mixed-replace;boundary=frame")
        except:
                pass

def spsdata(request):
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    ser.reset_input_buffer()
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            if line == "sps_start":
                data = {}
                while True:
                    line = ser.readline().decode('utf-8').rstrip()
                    if line == "sps_stop":
                        print(data)
                        return JsonResponse(data)
                        break
                    val = line.split(':', 2)
                    data[val[0]] = val[1].strip()

def sps(request):
    return render(request, 'sps.html')

def home(request):
    return render(request, 'home.html')
