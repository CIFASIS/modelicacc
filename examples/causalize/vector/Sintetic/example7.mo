model A
	Real b[10,10],a[10,10];
equation
	for i in 1:10, j in 1:10 loop
		a[i,j]+b[i,j]=23;
	end for;
	for i in 1:4, j in 1:3 loop
		a[i,j]=35;
	end for;
	for i in 5:7, j in 1:3 loop
		b[i,j]=5;
	end for;
	for i in 8:10, j in 1:3 loop
		a[i,j]=765;
	end for;
	for i in 1:4, j in 4:7 loop
		a[i,j]=75;
	end for;
	for i in 5:7, j in 4:7 loop
		b[i,j]=76;
	end for;
	for i in 8:10, j in 4:7 loop
		a[i,j]=1;
	end for;
	for i in 8:10, j in 1:4 loop
		b[j,i]=31;
	end for;
	for i in 5:7, j in 8:10 loop
		b[i,j]=371;
	end for;
	for i in 8:10, j in 8:10 loop
		a[i,j]=3141;
	end for;
end A;

// Solve is not working


