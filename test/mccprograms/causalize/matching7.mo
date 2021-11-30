model test7
  //Model with possibly wrong initial matchingTest
  constant Integer N=100000;
  Real x[N],a[N];
equation 
  for i in 2:N-1 loop
    a[i]=x[i];
    der(x[i])=a[i+1]-2*a[i]+a[i-1];
  end for;
  a[1]=x[1];
  a[N]=x[N];
  der(x[1])=1-2*a[1]+a[2];
  der(x[N])=a[N-1]-2*a[N];
end test7;
