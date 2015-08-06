model advection
	parameter Real alpha=0.5,mu=1000;
	constant Integer N = 1000;
	Real u[N];
	initial algorithm
		for i in 1:0.3*N loop
				u[i]:=1;
		 end for;
		
	equation
		der(u[1])=(-u[1]+1)*N-mu*u[1]*(u[1]-alpha)*(u[1]-1);
		for i in 2:N loop
		  der(u[i])=(-u[i]+u[i-1])*N-mu*u[i]*(u[i]-alpha)*(u[i]-1);
		end for;
end advection;
