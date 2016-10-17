model A
	Real a[13,11],b,c[10];
equation
	for i in 1:10, j in 2:11 loop
		a[j+2,i+1]=b;
	end for;
	for i in 1:13 loop
		a[i,1]=2677;
	end for;
	for i in 1:3 loop
		a[i,2]=17;
	end for;
	for i in 2:10, j in 5:7 loop
		a[j-4,i+1]=1777*b;
	end for;
	b=123;
	for i in 1:10 loop
		c[i]=17+a[i,4];
	end for;
end A;
