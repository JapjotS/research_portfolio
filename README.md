# Quantitative Research Portfolio

A collection of quantitative finance research and trading system implementations demonstrating expertise in algorithmic trading, market microstructure, and financial engineering.

## Repository Structure

```
research_portfolio/
├── research-notebooks/          # Jupyter notebooks for quant research
│   ├── 01_momentum_strategy.ipynb
│   ├── 02_mean_reversion_strategy.ipynb
│   ├── 03_volatility_clustering_research.ipynb
│   └── README.md
├── cpp-trading-engine/          # High-performance C++ trading engine
│   ├── include/
│   ├── src/
│   ├── tests/
│   └── README.md
└── README.md
```

## Research Notebooks

Well-documented Jupyter notebooks demonstrating quantitative trading research methodologies.

### Trading Strategies

| Notebook | Description | Key Concepts |
|----------|-------------|--------------|
| [Momentum Strategy](research-notebooks/01_momentum_strategy.ipynb) | Cross-sectional momentum with full backtest | 12-1 momentum, decile portfolios, Sharpe ratio |
| [Mean Reversion](research-notebooks/02_mean_reversion_strategy.ipynb) | Pairs trading using cointegration | Engle-Granger test, z-score signals, hedge ratios |

### Pure Research

| Notebook | Description | Key Concepts |
|----------|-------------|--------------|
| [Volatility Clustering](research-notebooks/03_volatility_clustering_research.ipynb) | Analysis of volatility dynamics | GARCH modeling, autocorrelation, regime detection |

### Example Results

#### Equity Curve (Momentum Strategy)
```
Typical performance metrics from backtests:
- Sharpe Ratio: 0.5 - 1.5
- Annual Return: 5-15%
- Max Drawdown: 15-30%
```

#### Volatility Clustering Findings
```
GARCH(1,1) parameters (typical equity):
- Alpha (shock reaction): 0.05 - 0.12
- Beta (persistence): 0.85 - 0.93
- Half-life of shocks: 10-30 days
```

## C++ Trading Engine

A production-quality, low-latency trading engine implementation in modern C++17.

### Features

- **Order Book**: Price-time priority with O(1) cancel
- **Matching Engine**: Supports Limit, Market, IOC, and FOK orders
- **Risk Management**: Position limits, order size limits, rate limiting
- **High Performance**: Microsecond-level latency

### Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Trading Engine                           │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐ │
│  │   Market    │  │    Risk     │  │         Order           │ │
│  │    Data     │──│  Manager    │──│   Management System     │ │
│  │   Handler   │  │             │  │                         │ │
│  └─────────────┘  └─────────────┘  └───────────┬─────────────┘ │
│                                                 │               │
│                                    ┌────────────▼────────────┐  │
│                                    │    Matching Engine      │  │
│                                    │  (Price-Time Priority)  │  │
│                                    └─────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

### Performance Benchmarks

| Operation | Mean Latency | 99th Percentile |
|-----------|-------------|-----------------|
| Order Insert | 2.3 μs | 5.1 μs |
| Order Cancel | 0.8 μs | 1.5 μs |
| Fill Notification | 0.5 μs | 0.9 μs |

### Building

```bash
cd cpp-trading-engine
mkdir build && cd build
cmake ..
make -j4
ctest --output-on-failure
```

## Technical Skills Demonstrated

### Programming Languages
- **Python**: NumPy, Pandas, Matplotlib, SciPy
- **C++17**: Modern C++, STL, template metaprogramming

### Quantitative Finance
- Factor models and portfolio construction
- Statistical arbitrage and pairs trading
- Time series analysis and volatility modeling
- Risk management and position sizing

### Software Engineering
- Test-driven development
- Performance optimization
- Clean architecture and SOLID principles
- Documentation and reproducibility

## Getting Started

### Research Notebooks

```bash
# Install dependencies
pip install numpy pandas matplotlib seaborn scipy jupyter

# Launch Jupyter
cd research-notebooks
jupyter notebook
```

### C++ Trading Engine

```bash
# Build the project
cd cpp-trading-engine
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
make

# Run tests
./test_order_book
./test_matching_engine
```

## License

MIT License - See [LICENSE](LICENSE) file for details.

## Contact

For questions or collaboration opportunities, please open an issue or reach out directly.