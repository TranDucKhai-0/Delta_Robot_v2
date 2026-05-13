function pr = passive_rotate(p, phi)
x = p(1); y = p(2); z = p(3);
xr =  x*cos(phi) + y*sin(phi);
yr = -x*sin(phi) + y*cos(phi);
pr = [xr, yr, z];
end
