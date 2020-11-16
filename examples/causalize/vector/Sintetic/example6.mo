model A
	Real b[10],a[10];
equation
	for i in 1:10 loop
		a[i]+b[i]=23;
	end for;
	for i in 1:3 loop
		a[i]=234;
	end for;
	for i in 4:7 loop
		b[i]=2;
	end for;
	for i in 8:10 loop
		a[i]=282;
	end for;
end A;

// Modify cancausalize
