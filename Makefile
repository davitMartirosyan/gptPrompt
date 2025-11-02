NAME=gpt
CXX=c++
CXX_FLAGS= -std=c++17 -Ivendor/httplib -Ivendor/nlohmann/
SRC= $(wildcard *.cpp)
OBJ= $(SRC:.cpp=.o)

all : $(NAME)


$(NAME) : $(OBJ)
	$(CXX) $(CXX_FLAGS) $(OBJ) -o $(NAME)

%.o : %.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

.PHONY: clean fclean all