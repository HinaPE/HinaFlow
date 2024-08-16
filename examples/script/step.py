(loss, incompressible_velocity, pressure_guess), gradient = eval_grad_v0(velocity_fit, p0)
remaining_divergence = field.divergence(incompressible_velocity)
velocity_fit -= gradient
