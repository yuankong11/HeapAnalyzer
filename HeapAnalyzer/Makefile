INCLUDE = -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux

test_agent: Sleep.class libagent.so
	export LD_LIBRARY_PATH=LD_LIBRARY_PATH:. && java -agentlib:agent= Sleep

libagent.so: agent.cpp agent.hpp
	gcc $^ ${INCLUDE} -Wall -Wno-deprecated -fPIC --share -o $@

libheapanalyzer.so: HeapAnalyzer.cpp
	gcc $^ ${INCLUDE} -Wall -Wno-deprecated -g -fPIC --share -o $@

Sleep.class: Sleep.java
	javac $^

Sleep: Sleep.class
	java Sleep

AttachAgent.class: AttachAgent.java
	javac -cp ${JAVA_HOME}/lib/tools.jar $^

attach: AttachAgent.class Sleep.class libheapanalyzer.so
	PID=$$(jps | grep Sleep | awk '{print $$1}') && \
	java -cp ${JAVA_HOME}/lib/tools.jar:. AttachAgent $${PID} ./libheapanalyzer.so

clean:
	@rm -f *.class
	@rm -f *.so
