CC = gcc

objects = car.o jpegutil.o

mycar:$(objects)
	$(CC)  -o mycar car.o jpegutil.o -lwiringPi -lopencv_core -lopencv_ml -lopencv_imgproc -lopencv_highgui -ljpeg 

car.o:car.c jpegutil.h
	$(CC) -c car.c

jpegutil.o: jpegutil.c jpegutil.h
	$(CC) -c jpegutil.c

clean:
	rm -rf $(objects) mycar



 
