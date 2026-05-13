function theta = IK(x0, y0, z0)
% IK trả RAD hoặc [] nếu vô nghiệm

p = delta_param();
rf = p.rf; re = p.re; a = p.a; k = p.k; phi = p.phi;

E0 = [x0, y0, z0];
theta = zeros(1,3);

for i = 1:3
    r  = passive_rotate(E0, phi(i));
    xr = r(1); yr = r(2); zr = r(3);

    xe = xr + k;
    ye = yr;
    ze = zr;

    d = sqrt((xe - a)^2 + ze^2);
    if d < 1e-12
        theta = []; return;
    end

    pval = (rf^2 - re^2 + ye^2 + d^2) / (2*d);
    if (pval > rf) || (pval < 0)
        theta = []; return;
    end

    inside = rf^2 - pval^2;
    if inside < 0
        theta = []; return;
    end
    h = sqrt(inside);

    xP = a + pval*(xe - a)/d;
    zP = pval*(ze)/d;

    vx = -ze;
    vz =  xe - a;

    x1 = xP + (h/d)*vx;
    z1 = zP + (h/d)*vz;

    x2 = xP - (h/d)*vx;
    z2 = zP - (h/d)*vz;

    if (x1 >= a) && (z1 <= 0)
        xJ = x1; zJ = z1;
    elseif (x2 >= a) && (z2 <= 0)
        xJ = x2; zJ = z2;
    else
        theta = []; return;
    end

    theta(i) = atan2(zJ, xJ - a);
end
end
