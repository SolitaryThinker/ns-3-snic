"""Example of generating correlated normally distributed random samples."""

import sys
import numpy as np
from scipy.linalg import eigh, cholesky
from scipy.stats import norm

import matplotlib.pyplot as plt
#import plot, show, axis, subplot, xlabel, ylabel, grid

num_samples = 400
num_phases = 20
points_per_phase = 50

def calculate_c(r):
    c = cholesky(r, lower=True)
    # evals, evecs = eigh(r)
    # # Construct c, so c*c^T = r.
    # c = np.dot(evecs, np.diag(np.sqrt(evals)))
    return c


def generate_points(means):
    x = np.zeros(shape=(means.shape[0], num_phases*points_per_phase))
    # x = np.zeros(shape=(1, means.shape[0]*points_per_phase)
    for idx, mean in enumerate(means):
        for phaseIdx, p in enumerate(mean):
            x[idx][phaseIdx*points_per_phase:phaseIdx*points_per_phase+points_per_phase] = np.fmax(0,norm.rvs(loc=p, scale=5, size=(1, points_per_phase)))
    return x


def calc(average, std, num_phases, ppp):
    std1 = std
    std2 = std
    print('avg: ', average);
    print('std: ', std);
    print('num_phases: ', num_phases);
    print('ppp: ', ppp);
    var = pow(std, 2)
    cov = .99
    r = np.array([
            [ var, 0.99],
            [ 0.99, var]
            # [ 1.00, 2.00, 4.00]
        ])
    # correlation coefficient =
    corr=cov/(std1*std2)
    print('correlation coeff:', corr)
    sys.exit(0);


# Choice of cholesky or eigenvector method.
#method = 'eigenvectors'
# calc(63, 20, 20, 20)


# The desired covariance matrix.
r = np.array([
        [ 1.0, 0.99],
        [ 0.99, 1.0]
        # [ 1.00, 2.00, 4.00]
    ])
# r = np.array([
        # [  3.40, -2.75, -2.00],
        # [ -2.75,  5.50,  1.50],
        # [ -2.00,  1.50,  1.25]
    # ])

# Generate samples from three independent normally distributed random
# variables (with mean 0 and std. dev. 1).
# x = norm.rvs(size=(2, num_samples))

num_traces = 3
fig, axes = plt.subplots(2,num_traces)
fig.subplots_adjust(wspace=0.5, hspace=.5)

locs = [25, 33, 50, 75, 100]

for idx, loc in enumerate(locs):
    std = 35
    over_100s = []
    covs = []
    averages = []
    ind_averages = [[] for i in range(num_traces)]
    # print(ind_averages)
    x_averages=[]
    x2_averages=[]
    means = norm.rvs(loc=loc, scale=std, size=(num_traces, num_phases))
    x = np.zeros(shape=(num_traces, num_phases*points_per_phase))
    x = generate_points(np.fmax(0, means))

    # for cov in np.array(range(-99,100))/10.0:
    for correlation in np.array(range(-99,100))/100.0:
        std = .6
        # var1 = pow(std,2)
        var1 = 1
        # var2 = pow(std,2)
        cov = correlation * std*std
        # cov2 = correlation * 2*2
        # print(correlation, cov)
        # correlation = cov
        # r = np.array([
                # [ var1, cov],
                # [ cov, var1]
            # ])
        r = np.array([
                [ var1, cov, cov],
                [ cov, var1, cov],
                [ cov, cov, var1]
            ])
        c = calculate_c(r)
        correlated_means = np.dot(c, means)
        correlated_x = generate_points(correlated_means)
        # summed = correlated_x[0]+correlated_x[1]
        summed = np.sum(correlated_x, axis=0)
        avg = np.average(summed)
        averages.append(avg)
        over = (summed > 100)
        # print(correlated_x[0])
        over_count = np.count_nonzero(over == True)
        covs.append(correlation)
        over_100s.append(over_count / (num_phases*points_per_phase))
        # x_averages.append(np.average(correlated_x[0]))
        # x2_averages.append(np.average(correlated_x[1]))
        for t in range(num_traces):
            ind_averages[t].append(np.average(correlated_x[t]))
        # print(r)

    ax = axes[0,0]
    ax.plot(covs, over_100s, label='{}Gbps'.format(loc))
    ax.set_ylabel('% of workload runtime')
    ax.set_xlabel('correlation coefficient')
    ax.set_title('% of time over 100Gbps (summed)')
    ax.set_xticks(np.array(range(-9,10, 3))/10.0)
    # ax.axis('equal')
    ax.grid(True)

    ax = axes[0,1]
    ax.plot(covs, averages, label='{}Gbps'.format(loc))
    ax.set_ylabel('bandwidth (Gbps)')
    ax.set_xlabel('correlation coefficient')
    ax.set_title('summed averages bandwidth')
    # ax.axis('equal')
    ax.grid(True)

    for t in range(num_traces):


        ax = axes[1,t]
        ax.plot(covs, ind_averages[t])
        ax.set_ylabel('bandwidth (Gbps)')
        ax.set_xlabel('correlation coefficient')
        ax.set_title('Average bandwidth of trace {}'.format(t+1))
        # ax.axis('equal')
        ax.grid(True)

    # ax = axes[1,1]
    # ax.plot(covs, x2_averages)
    # ax.set_ylabel('bandwidth (Gbps)')
    # ax.set_xlabel('correlation coefficient')
    # ax.set_title('Average bandwidth of trace 2')
    # # ax.axis('equal')
    # ax.grid(True)
axes[0,1].legend()

# print(np.average(x[0]+x[1]))
# print(np.average(correlated_x[0]+correlated_x[1]))


# plt.savefig('link_util (2 hosts).pdf', bbox_inches='tight')
plt.savefig('3_hosts.png')
# plt.show()
