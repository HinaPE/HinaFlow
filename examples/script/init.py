from phi.torch.flow import *

DOMAIN = dict(x=80, y=64)
LEFT = StaggeredGrid(Box(x=(-INF, 40), y=None), 0, **DOMAIN)
RIGHT = StaggeredGrid(Box(x=(40, INF), y=None), 0, **DOMAIN)
TARGET = RIGHT * StaggeredGrid(lambda x: math.exp(-0.5 * math.vec_squared(x - (50, 10), 'vector') / 32**2), 0, **DOMAIN) * (0, 2)


def loss(v0, p0):
    v1, p = fluid.make_incompressible(v0 * LEFT, solve=Solve('CG-adaptive', 1e-5, x0=p0))
    return field.l2_loss((v1 - TARGET) * RIGHT), v1, p

eval_grad_v0 = field.functional_gradient(loss, 'v0', get_output=True)
p0 = gradient = incompressible_velocity = remaining_divergence = None
velocity_fit = StaggeredGrid(Noise(), 0, **DOMAIN) * 0.1 * LEFT
