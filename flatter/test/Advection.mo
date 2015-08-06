model Advection
  constant Integer N = 30;
  constant Real alpha = 0.5, mu = 1000;

  connector Interface
    Real u;
  end Interface;

  class Segment
    Interface prev, next;
    Real u;
  equation
    der(u) = (prev.u - u) * N - mu * u * (u - alpha) * (u - 1);
    next.u = u;
  end Segment;

  Segment seg[N];
equation
  for i in 2:N loop
    connect(seg[i].prev, seg[i - 1].next);
  end for;
  seg[1].prev.u = 1;
end Advection;