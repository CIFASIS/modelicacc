model test2
  //Model without algebraic loops containing connections between different indices in one direction
  constant Integer N=1000000;
  Real x[N],a[N];
equation
  a[1]=x[1];
  der(x[1])=1-a[1];
  for i in 2:N loop
    a[i]=a[i-1]-x[i];
    der(x[i])=a[i];
  end for;  
end test2;
