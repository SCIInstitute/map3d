      program ftest1
      implicit none
      include 'fi_f.h'
      integer result
      integer*2 type
      character*50 name, label

      result = FI_FInit(1)

      result = FI_FFirst(type)
      result = FI_FTypeToName(type, name)
      result = FI_FGetLabel(type, label)
      write(*,*) 'type=', type
      write(*,*) 'name=', name
      write(*,*) 'label=', label
      do while(FI_FNext(type))
          result = FI_FTypeToName(type, name)
          result = FI_FGetLabel(type, label)
          write(*,*) 'type=', type
          write(*,*) 'name=', name
          write(*,*) 'label=', label
      enddo

      write(*,*) '================'
      write(*,*) 'name=', FI_PON_NAME
      result = FI_FNameToType(FI_PON_NAME, type)
      write(*,*) 'result=', result
      result = FI_FGetLabel(type, label)
      write(*,*) 'label=', label

      end
