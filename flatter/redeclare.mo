model Hola
	class Pin3
    		Real w;
    		Real j;
		class Pin5
			Real j;
		equation
			j = 5;
		end Pin5;
  	equation
    		w = 100 + j;
  	end Pin2;

	class Pin4
    		Real w;
    		Real j;
  	equation
		Pin.Pin5 www;
    		w = 100 + j;
  	end Pin2;


  	class A
     		replaceable class Pin
      			Real w;
   	 	end Pin;
	    		
		replaceable class Pin2
      			Real w;
   	 	end Pin;

  	end A; 
    
	A cl(redeclare  class Pin  = Pin3, redeclare class Pin2 = Pin4);
end Hola;
