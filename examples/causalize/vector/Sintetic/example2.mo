model A
	Real b[10,20];
equation
	for i in 1:10, j in 1:20 loop
		b[i,j] + b[j,i]=1345;
	end for;
end A;

// Multiple usages of a same vector with different index usages in a same for equation not supported
