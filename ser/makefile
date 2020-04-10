INC += -I./
LIB +=  -ljson -levent -lpthread -lhiredis
OBJ_DIR = ./obj
OBJ =  $(OBJ_DIR)/Socket.o  $(OBJ_DIR)/sys.o $(OBJ_DIR)/main.o $(OBJ_DIR)/mytime.o $(OBJ_DIR)/redis.o 
OBJ_EXT = .o
CXX_EXT = .cpp


main : $(OBJ)
	@echo
	@echo "Linking..."
	g++ -std=c++11 -o main $(OBJ) $(LIB)
	@echo

$(OBJ_DIR)/%$(OBJ_EXT) : %$(CXX_EXT)
	@echo
	@echo "Compiling $< ==> $@..."
	@-mkdir -p $(OBJ_DIR)
	g++ -std=c++11 -c $< -o $@

cc:
	@echo
	@echo "Removeing..."
	-rm -rf $(OBJ_DIR) ./bin main
	@echo