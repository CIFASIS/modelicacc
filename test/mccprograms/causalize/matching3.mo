model test3
  //Model with small algebraic loops and without connections between different indices
  constant Integer N=1000000;
  Real x[N],a[N],b[N];
equation
  for i in 1:N loop
    a[i]=2*x[i]-b[i];
    a[i]=2*b[i]-x[i];
    der(x[i])=1-a[i];
  end for;  
end test3;
