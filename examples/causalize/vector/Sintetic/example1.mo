model A
	Real a,b[10,20];
equation
	b[3,5]=12;
	for i in 1:10, j in 1:20 loop
		b[i,j]+a=1345;
	end for;
end A;

// Cauzalization working. No solving.
