function p_rot = passive_rotate(p, phi)
    % p   : [1x3] hoặc [3x1]  (m)
    % phi : rad
    
    x = p(1);
    y = p(2);
    
    xr =  x*cos(phi) + y*sin(phi);
    yr = -x*sin(phi) + y*cos(phi);
    
    p_rot = [xr; yr; p(3)];   % column vector
end
