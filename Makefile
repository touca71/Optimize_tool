CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = test_program

OBJS = main.o LinearProgramModel.o SimplexSolver.o 
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)


main.o: main.cpp LinearProgramModel.h SimplexSolver.h
	$(CXX) $(CXXFLAGS) -c main.cpp

LinearProgramModel.o: LinearProgramModel.cpp LinearProgramModel.h
	$(CXX) $(CXXFLAGS) -c LinearProgramModel.cpp

SimplexSolver.o: SimplexSolver.cpp SimplexSolver.h LinearProgramModel.h
	$(CXX) $(CXXFLAGS) -c SimplexSoslver.cpp

clean:
	rm -f $(TARGET) $(OBJS)