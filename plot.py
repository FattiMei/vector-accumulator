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
    throughput = (nbytes / 2**30) / time

    return name, nbytes, throughput


def process_cache_data(report):
    return [
        cache['size']
        for cache in report['context']['caches']
        if cache['type'] in ('Data', 'Unified')
    ]


def parse(json_path):
    result = {}

    with open(json_path, 'r') as file:
        report = json.load(file)

    for bench in report['benchmarks']:
        name, nbytes, throughput = process_benchmark(bench)

        try:
            result[name]['bytes'].append(nbytes)
            result[name]['throughput'].append(throughput)

        except KeyError:
            result[name] = {}
            result[name]['bytes'] = [nbytes]
            result[name]['throughput'] = [throughput]

    for name, data in result.items():
        result[name]['bytes'] = np.array(data['bytes'])
        result[name]['throughput'] = np.array(data['throughput'])

    return result, process_cache_data(report)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python plot.py <json report>")
        sys.exit(1)

    results, cache = parse(sys.argv[1])


    # ---------------------------- scalability plot ---------------------------
    plt.title("dot product scalability")
    plt.xlabel("size [bytes]")
    plt.ylabel("throughput [Gb/s]")

    for name, data in results.items():
        plt.semilogx(data['bytes'], data['throughput'], label=name)

    for c in cache:
        plt.axvline(c, color='black', linewidth=1)

    plt.legend(framealpha=1)

    try:
        plt.show()

    except:
        print("Can't plot on screen, saving to file")
        plt.savefig('dot_scalability.svg', format='svg')
        plt.savefig('dot_scalability.png', format='png')


    # -------------------------------speedup plot ------------------------------
    reference = results['dot_naive']['throughput']

    plt.title("dot product speedup")
    plt.xlabel("size [bytes]")
    plt.ylabel("speedup")

    for name, data in results.items():
        plt.semilogx(data['bytes'], data['throughput'] / reference, label=name)

    for c in cache:
        plt.axvline(c, color='black', linewidth=1)

    plt.legend(framealpha=1)

    try:
        plt.show()

    except:
        plt.savefig('dot_speedup.svg', format='svg')
        plt.savefig('dot_speedup.png', format='png')
