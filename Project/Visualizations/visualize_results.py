import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

results_df = pd.read_csv("../Statistics/service_time_trial.csv")
print(results_df.head())

# plot servive_time_history vs service_time_goal
results_df.plot(y=['service_time_history', 'service_time_goal'],
                style=['-', '--'])
plt.ylabel('Service Time (microseconds)')
plt.xlabel('Task')
plt.title('Service Time')
plt.show()

# plot relative error between  service_time_goal and service_time_history
y1 = np.array(results_df['service_time_history'])
y2 = np.array(results_df['service_time_goal'])

relative_error = (y1-y2)/y2

plt.plot(relative_error)
plt.ylabel('Relative Error')
plt.xlabel('Task')
plt.title('Service Time Error')
plt.show()

# plot parallelism degree vs max nw
results_df.plot(y=['active_workers_history', 'max_nw'],
                style=['-', '--'])
plt.ylabel('Workers')
plt.xlabel('Tasks')
plt.title('Parallelism Degree')
plt.show()

# plot emitter, workers and collector elapsed time history
results_df.plot(y=['emitter_elapsed_time_history','workers_elapsed_time_history','collector_elapsed_time_history'],
                style=['-', '--', ':'])
plt.ylabel('Elapsed Time')
plt.xlabel('Tasks')
plt.title('Elapsed Time History')
plt.show()
