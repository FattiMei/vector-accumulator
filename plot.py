import re
import sys
import json
import numpy as np
import matplotlib.pyplot as plt


def get_time_unit(unit):
    return {
        's':  1e0,
        'ms': 1e-3,
        'us': 1e-6,
        'ns': 1e-9
    }[unit]


def process_benchmark(benchmark):
    name, n = re.findall(
        r'<(.*)>/(.*)',
        benchmark['name']
    )[0]

    nbytes = int(n)
    time = benchmark['real_time'] * get_time_unit(benchmark['time_unit'])

    # throughput is in gigabytes per seconds
    throughput = (nbytes / 2**30) / time

    return name, nbytes, throughput


def process_cache_data(report):
    return [
        cache['size']
        for cache in report['context']['caches']
        if cache['type'] in ('Data', 'Unified')
    ]


def collect_results(report):
    results = {}

    for run in report['benchmarks']:
        name, nbytes, throughput = process_benchmark(run)

        if name in results:
            results[name]['bytes'].append(nbytes)
            results[name]['throughput'].append(throughput)
        else:
            results[name] = {}
            results[name]['bytes'] = [nbytes]
            results[name]['throughput'] = [throughput]

    for name, data in results.items():
        results[name]['bytes'] = np.array(data['bytes'])
        results[name]['throughput'] = np.array(data['throughput'])

    return results


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python plot.py <json report>")
        sys.exit(1)

    with open(sys.argv[1], 'r') as file:
        report = json.load(file)

    cache         = process_cache_data(report)
    experiment    = report['context']['experiment']
    floating_type = report['context']['floating_type']
    memory_type   = report['context']['memory_type']
    results       = collect_results(report)

    # ---------------------------- scalability plot ---------------------------
    plt.figure(1)
    plt.title(f"{experiment} throughput")
    plt.xlabel("size [bytes]")
    plt.ylabel("throughput [GB/s]")

    for name, data in results.items():
        # this is not a general transformation and is here only because I can't export the floating_type in the benchmark name
        corrected_name = name.replace("floating", floating_type)
        plt.semilogx(data['bytes'], data['throughput'], label=corrected_name, base=2)

    for c in cache:
        plt.axvline(c, color='black', linewidth=1)

    plt.legend(framealpha=1)

    # -------------------------------speedup plot ------------------------------
    reference_experiment_name = [name for name in results.keys() if name.find('naive') != -1][0]
    reference = results[reference_experiment_name]['throughput']

    plt.figure(2)
    plt.title(f"{experiment} speedup")
    plt.xlabel("size [bytes]")
    plt.ylabel("speedup")

    for name, data in results.items():
        corrected_name = name.replace("floating", floating_type)
        plt.semilogx(data['bytes'], data['throughput'] / reference, label=corrected_name, base=2)

    for c in cache:
        plt.axvline(c, color='black', linewidth=1)

    plt.legend(framealpha=1)

    plt.show()
