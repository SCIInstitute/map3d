all: prereqs
	@cd fi && $(MAKE) MASTER_FLAGS_SET=yes
	@cd graphicsio && $(MAKE) MASTER_FLAGS_SET=yes
	@cd cutil && $(MAKE) MASTER_FLAGS_SET=yes
	@cd fids && $(MAKE) MASTER_FLAGS_SET=yes
	@cd gdbmp && $(MAKE) MASTER_FLAGS_SET=yes
	@cd tsdflib && $(MAKE) MASTER_FLAGS_SET=yes
	@cd MatlabIO && $(MAKE) MASTER_FLAGS_SET=yes
	@cd gfilelib && $(MAKE) MASTER_FLAGS_SET=yes
	@cd map3d && $(MAKE) MASTER_FLAGS_SET=yes 
  
clean:
	@cd fi && $(MAKE) clean
	@cd graphicsio && $(MAKE) clean
	@cd cutil && $(MAKE) clean
	@cd fids && $(MAKE) clean
	@cd gdbmp && $(MAKE) clean
	@cd tsdflib && $(MAKE) clean
	@cd MatlabIO && $(MAKE) clean
	@cd gfilelib && $(MAKE) clean
	@cd map3d && $(MAKE) clean
  
.PHONY: prereqs
prereqs: 
	@mkdir -p lib
