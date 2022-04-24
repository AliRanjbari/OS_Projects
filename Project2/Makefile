

all: meanCalculator.out class_handler.out student_handler.out course_handler.out

meanCalculator.out : meanCalculator.cpp
	g++ -std=c++17 meanCalculator.cpp -o meanCalculator.out

 class_handler.out : class_handler.cpp
	g++ -std=c++17 class_handler.cpp -o class_handler.out

student_handler.out : student_handler.cpp
	g++ -std=c++17 student_handler.cpp -o student_handler.out

course_handler.out: course_handler.cpp
	g++ -std=c++17 course_handler.cpp -o course_handler.out

clean:
	rm *.out