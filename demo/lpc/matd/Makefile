
MOD1 = tcpip
MOD2 = matd

LPLC = ../../..
HANDLER = $(LPLC)/lib/lpc
DEST = $(LPLC)/tools


all:	$(DEST)/lib/$(MOD1).so $(DEST)/lib/$(MOD2).so $(EXECUTE)/run/handler 

$(EXECUTE)/run/handler:
		cd $(HANDLER) ; make

$(DEST)/lib/$(MOD1).so: 
		cd $(LPLC)/comm/tcpip_lib ; make


$(DEST)/lib/$(MOD2).so:	
		cd $(LPLC)/service/matd_lib ; make

clean:
	cd $(HANDLER) ; make clean
	cd $(LPLC)/comm/tcpip_lib ; make clean
	cd $(LPLC)/service/matd_lib ; make clean
