model A
    connector  Pin
		Real v;
		flow Real i;
    end Pin;
    Pin a[10],b[10],c[10];
equation
	for i in 1:10 loop
		connect(a[i],b[i ]);	
	end for;

	for i in 1:5 loop
		connect(b[i+3],c[i]);	
	end for;	
	a[1].v = a[5].v;
end A;

