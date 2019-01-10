EXECUTEFILE=dd
OBJECT=main.o file.o  notiferLine.o

$(EXECUTEFILE):$(OBJECT)
	g++ -o $(EXECUTEFILE) $(OBJECT)
	
%.o:%.cpp
	g++ -c $< -o $@
	
.PHONY:clean
clean:
	-rm -f $(EXECUTEFILE) $(OBJECT)
