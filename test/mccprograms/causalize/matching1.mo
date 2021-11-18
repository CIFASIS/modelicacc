model test1
  //Model without algebraic loops and without connections between different indices
  constant Integer N=100000000;
  Real x[N],a[N];
equation
  for i in 1:N loop
    a[i]=x[i];
    der(x[i])=1-a[i];
  end for;  
end test1;
