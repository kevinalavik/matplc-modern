DIRECTORIES =	arguments/arguments.so \
		chaser/chaser.so \
		das08/logic.so \
		echo_network/echo_managed.so \
		echo_serial/echo_managed.so \
		emailer/emailer.so \
		ladder_basic/ladder_test.so \
		ladder_chaser \
		matd \
		messages/test.so \
		test/test.so 

all: $(DIRECTORIES)


arguments/arguments.so:
	cd arguments; make

chaser/chaser.so:
	cd chaser; make

das08/logic.so:
	cd das08; make

echo_network/echo_managed.so:
	cd echo_network; make

echo_serial/echo_managed.so:
	cd echo_serial; make

emailer/emailer.so:
	cd emailer; make

ladder_basic/ladder_test.so:
	cd ladder_basic; make

ladder_chaser:
	cd ladder_chaser; make

matd:
	cd matd; make

messages/test.so:
	cd messages; make

test/test.so:
	cd test; make


clean:
	cd arguments; make clean
	cd chaser; make clean
	cd das08; make clean
	cd echo_network; make clean
	cd echo_serial; make clean
	cd emailer; make clean
	cd ladder_basic; make clean
	cd ladder_chaser; make clean
	cd matd; make clean
	cd messages; make clean
	cd test; make clean
	rm -f */linuxplc.log


