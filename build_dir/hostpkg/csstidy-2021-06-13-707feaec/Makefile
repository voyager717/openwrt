CPPFLAGS ?= -Icsstidy
CFLAGS ?= -O2

OBJ:=$(addsuffix .o,$(basename $(wildcard csstidy/*.cpp)))

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

csstidy/csstidy: $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) csstidy/csstidy
