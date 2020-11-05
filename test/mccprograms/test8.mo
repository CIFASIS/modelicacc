model A
    class Pin
		Real v;
		flow Real i;	
    end Pin;
    Pin a[5, 5, 5],b[5, 5, 5],c[5],r;
	class BB
		Real tt;
	end BB;
	BB qw;
    Real den = 3;
equation
	for i in 1:5 loop
      for j in 1:5 loop
         for k in 1:5 loop
		   connect(a[i, j, k],b[i, j, k]);	
         end for;
      end for;
	end for;


	for i in 1:3 loop
		connect(c[i], c[i]);	
	end for;	
end A;

