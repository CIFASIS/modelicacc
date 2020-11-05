model A
    connector  Pin
		Real v;
		flow Real i;
    end Pin;
    Pin a[10],b[10],c[10];
    Integer z[10] = {1,2,3,4,5,6,7,8,9,10};
    Integer m = 3;
    type Vegie = enumeration(Potatoe "Potatooo!", Orange "Hey apple!", Apple "Waaat?");
    Real h[Vegie], j[Vegie];
equation
	for i in 1:10 loop
		connect(a[i],b[i]);	
        //connect(h[Potatoe], j[Orange]);
	end for;

    connect(a[:], b[:]);

	for i in 1:5 loop
		connect(b[i+3],c[i]);	
	end for;	
	a[1].v = a[5].v;
end A;

