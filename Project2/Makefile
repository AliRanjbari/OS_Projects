

all: meanCalculator.out class_handler.out student_handler.out

meanCalculator.out : meanCalculator.cpp
	g++ -std=c++17 meanCalculator.cpp -o meanCalculator.out

 class_handler.out : class_handler.cpp
	g++ -std=c++17 class_handler.cpp -o class_handler.out

student_handler.out : student_handler.cpp
	g++ -std=c++17 student_handler.cpp -o student_handler.out



clean:
	rm *.out