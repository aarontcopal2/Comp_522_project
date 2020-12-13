import numpy as np
from matplotlib.pylab import plt #load plot library
import matplotlib as mpl
import csv

# data
threads=[1,2,4,8,16,32]
splay_benchmark = []
sol_benchmark = []


with open('output.csv', 'r') as fd:
    reader = csv.reader(fd)
    for idx, row in enumerate(reader):
        if idx < 5:
            splay_benchmark.append(row)
        else:
            sol_benchmark.append(row)
            
splay_benchmark = [list( map(float,i) ) for i in splay_benchmark]
sol_benchmark = [list( map(float,i) ) for i in sol_benchmark]


# plot processing
font = {'size'   : 8}
mpl.rc('font', **font)

fig = plt.figure()

ax = plt.subplot2grid((3,4), (0, 0), colspan=2)
fontsize = 10
ax.plot(threads,splay_benchmark[0],c='b',marker="^",ls='--',label='Lock-based splay tree',fillstyle='none')
ax.plot(threads,sol_benchmark[0],c='m',marker="o",ls='--',label='Lock-Free hash table',fillstyle='none')
ax.set_title("50% inserts, 0% finds, 50% deletes",fontsize=fontsize)

ax = plt.subplot2grid((3,4), (0, 2), colspan=2)
ax.plot(threads,splay_benchmark[1],c='b',marker="^",ls='--',label='Lock-based splay tree',fillstyle='none')
ax.plot(threads,sol_benchmark[1],c='m',marker="o",ls='--',label='Lock-Free hash table',fillstyle='none')
ax.set_title("33% inserts, 33% finds, 33% deletes",fontsize=fontsize)

ax = plt.subplot2grid((3,4), (1, 0), colspan=2)
ax.plot(threads,splay_benchmark[2],c='b',marker="^",ls='--',label='Lock-based splay tree',fillstyle='none')
ax.plot(threads,sol_benchmark[2],c='m',marker="o",ls='--',label='Lock-Free hash table',fillstyle='none')
ax.set_title("25% inserts, 50% finds, 25% deletes",fontsize=fontsize)

ax = plt.subplot2grid((3,4), (1, 2), colspan=2)
ax.plot(threads,splay_benchmark[3],c='b',marker="^",ls='--',label='Lock-based splay tree',fillstyle='none')
ax.plot(threads,sol_benchmark[3],c='m',marker="o",ls='--',label='Lock-Free hash table',fillstyle='none')
ax.set_title("15% inserts, 70% finds, 15% deletes",fontsize=fontsize)

ax = plt.subplot2grid((3,4), (2, 1), colspan=2)
ax.plot(threads,splay_benchmark[4],c='b',marker="^",ls='--',label='Lock-based splay tree',fillstyle='none')
ax.plot(threads,sol_benchmark[4],c='m',marker="o",ls='--',label='Lock-Free hash table',fillstyle='none')
ax.set_title("5% inserts, 90% finds, 5% deletes",fontsize=fontsize)
ax.set
fig.tight_layout()
handles, labels = ax.get_legend_handles_labels()
fig.legend(handles, labels,bbox_to_anchor=(0.73,0.15),loc='lower left',fontsize=7.5)


# plt.show()
plt.savefig('performance_charts.pdf')