model A
    class Pin
		Real v;
	
    end Pin;
    Pin a[10],b[10];
equation
	for i in 1:3 loop
		connect(a[i],b[i ]);	
	end for;

	for i in 1:3 loop
		connect(b[i],a[i+3 ]);	
	end for;
end A;

