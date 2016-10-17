model A
	Real b[10,20];
equation
	for i in 1:10, j in 1:20 loop
		b[i,j] + b[j,i]=1345;
	end for;
end A;

// Remove duplicates!!!
