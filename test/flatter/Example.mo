// Este es el modelo original. Sin exportar el modelo completo
model Example
  Modelica.Blocks.Math.Add add1 annotation(Placement(visible = true, transformation(origin = {-5, 55}, extent = {{-15, -15}, {15, 15}}, rotation = 0)));
  Modelica.Blocks.Math.Product product1 annotation(Placement(visible = true, transformation(origin = {-60, 20}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
  Modelica.Blocks.Sources.Sine sine1 annotation(Placement(visible = true, transformation(origin = {-60, 80}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
  Modelica.Blocks.Sources.Pulse pulse1 annotation(Placement(visible = true, transformation(origin = {-80, -20}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
  Modelica.Blocks.Sources.Cosine cosine1 annotation(Placement(visible = true, transformation(origin = {-80, 60}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
equation
  connect(cosine1.y, product1.u1) annotation(Line(points = {{-69, 60}, {-72.4324, 60}, {-72.4324, 25.2252}, {-72.4324, 25.2252}}));
  connect(pulse1.y, product1.u2) annotation(Line(points = {{-69, -20}, {-71.71169999999999, -20}, {-71.71169999999999, 13.6937}, {-71.71169999999999, 13.6937}}));
  connect(product1.y, add1.u2) annotation(Line(points = {{-49, 20}, {-23.4234, 20}, {-23.4234, 45.7658}, {-23.4234, 45.7658}}));
  connect(sine1.y, add1.u1) annotation(Line(points = {{-49, 80}, {-23.4234, 80}, {-23.4234, 64.50449999999999}, {-23.4234, 64.50449999999999}}));
  connect(product1.y, add1.u2) annotation(Line(points = {{-49, 20}, {-20.1802, 20}, {-20.1802, 46.4865}, {-20.1802, 46.4865}}));
  annotation(Icon(coordinateSystem(extent = {{-100, -100}, {100, 100}}, preserveAspectRatio = true, initialScale = 0.1, grid = {2, 2})), Diagram(coordinateSystem(extent = {{-100, -100}, {100, 100}}, preserveAspectRatio = true, initialScale = 0.1, grid = {2, 2})));
end Example;
