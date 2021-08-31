INCLUDE = -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux

test_agent: Sleep.class libagent.so
	export LD_LIBRARY_PATH=LD_LIBRARY_PATH:. && java -agentlib:agent= Sleep

libagent.so: agent.cpp agent.hpp
	gcc $^ ${INCLUDE} -Wall -Wno-deprecated -fPIC --share -o $@

libheapanalyzer.so: HeapAnalyzer.cpp
	gcc $^ ${INCLUDE} -Wall -Wno-deprecated -g -fPIC -lstdc++ --share -o $@

Sleep.class: Sleep.java
	javac $^

Sleep: Sleep.class
	java Sleep

ByteHolderTest.class: ByteHolderTest.java
	javac $^

ByteHolderTest: ByteHolderTest.class
	java ByteHolderTest

ByteHolderTestDBG: ByteHolderTest.class
	java -Xdebug -Xrunjdwp:transport=dt_socket,server=y,address=8000 ByteHolderTest

AttachAgent.class: AttachAgent.java
	javac -cp ${JAVA_HOME}/lib/tools.jar $^

turn_off_ptrace_scope:
	echo 0 > /proc/sys/kernel/yama/ptrace_scope

NAME = ByteHolderTest

dacapo: dacapo-9.12-MR1-bach.jar
	java -jar $^ h2 -s small

jmap:
	PID=$$(jps | grep ${NAME} | awk '{print $$1}') && \
	jmap -histo:live $${PID} > jmap.txt

jmap_all:
	PID=$$(jps | grep ${NAME} | awk '{print $$1}') && \
	jmap -histo $${PID} > jmap.txt

attach: AttachAgent.class libheapanalyzer.so
	PID=$$(jps | grep ${NAME} | awk '{print $$1}') && \
	java -cp ${JAVA_HOME}/lib/tools.jar:. AttachAgent $${PID} ./libheapanalyzer.so

clean:
	@rm -f *.class
	@rm -f *.so
