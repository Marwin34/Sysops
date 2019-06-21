GCC=gcc -Wall
FILTER_SRC=filter.c
FILTER_H=filter.h
IMAGE_SRC=image.c
IMAGE_H=image.h
MAIN_SRC=main.c
MAIN_OUT=main
FILTER_GENERATOR_SRC=filter_generator.c
FILTER_GENERATOR_OUT=filter_generator
LIB=-lpthread
REPORT_SCRPIT=report.sh


build: $(FILTER_H) $(FILTER_SRC) $(IMAGE_H) $(IMAGE_SRC) $(MAIN_SRC)
	$(GCC) $(IMAGE_SRC) $(FILTER_SRC) $(MAIN_SRC) -o $(MAIN_OUT) $(LIB)
	$(GCC) $(IMAGE_SRC) $(FILTER_SRC) $(FILTER_GENERATOR_SRC) -o $(FILTER_GENERATOR_OUT)

clean:
	rm -rf $(MAIN_OUT) $(FILTER_GENERATOR_OUT)
	rm -rf images_out/*

generate_report: $(REPORT_SCRPIT)
	./$(REPORT_SCRPIT)

test:
	@echo "TESTS FOR BLOCK OPTION"
	@echo "1 THREAD"
	@./main 1 0 images/lena.ascii.pgm generated_filters/filter_3.txt images_out/test_10.pgm
	@echo "2 THREADS"
	@./main 2 0 images/lena.ascii.pgm generated_filters/filter_3.txt images_out/test_20.pgm
	@echo "4 THREADS"
	@./main 4 0 images/lena.ascii.pgm generated_filters/filter_3.txt images_out/test_40.pgm
	@echo "8 THREADS"
	@./main 8 0 images/lena.ascii.pgm generated_filters/filter_3.txt images_out/test_80.pgm

	@echo "TESTS FOR INTERLEAVED OPTION"
	@echo "1 THREAD"
	@./main 1 1 images/lena.ascii.pgm generated_filters/filter_3.txt images_out/test_11.pgm
	@echo "2 THREADS"
	@./main 2 1 images/lena.ascii.pgm generated_filters/filter_3.txt images_out/test_21.pgm
	@echo "4 THREADS"
	@./main 4 1 images/lena.ascii.pgm generated_filters/filter_3.txt images_out/test_41.pgm
	@echo "8 THREADS"
	@./main 8 1 images/lena.ascii.pgm generated_filters/filter_3.txt images_out/test_81.pgm
	

