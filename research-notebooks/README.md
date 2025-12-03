# Research Notebooks

This directory contains well-documented Jupyter notebooks demonstrating quantitative trading research and strategy development.

## Contents

### Trading Strategies

#### 1. [Momentum Strategy](01_momentum_strategy.ipynb)
A complete implementation of a cross-sectional momentum strategy, following the full pipeline:
- **Data Acquisition**: Synthetic price data generation with realistic market characteristics
- **Feature Engineering**: Multi-horizon returns and rolling volatility calculations
- **Signal Construction**: 12-1 momentum signal with cross-sectional ranking
- **Backtesting**: Portfolio construction with transaction cost modeling
- **Performance Analysis**: Risk-adjusted metrics, equity curves, drawdowns

**Key Concepts**: Cross-sectional momentum, decile portfolios, turnover analysis, Sharpe ratio

#### 2. [Mean Reversion Strategy](02_mean_reversion_strategy.ipynb)
A statistical arbitrage pairs trading strategy based on cointegration:
- **Pair Selection**: Correlation-based pre-filtering
- **Cointegration Testing**: Engle-Granger two-step method
- **Signal Construction**: Z-score based entry/exit signals
- **Risk Management**: Stop-loss implementation
- **Performance Analysis**: Individual pair decomposition

**Key Concepts**: Cointegration, hedge ratios, half-life of mean reversion, z-scores

### Pure Research

#### 3. [Volatility Clustering Analysis](03_volatility_clustering_research.ipynb)
A comprehensive research study on volatility dynamics in financial markets:
- **Literature Review**: Theoretical background on volatility clustering
- **Empirical Analysis**: Return distribution analysis, autocorrelation functions
- **GARCH Modeling**: Maximum likelihood estimation of GARCH(1,1) parameters
- **Cross-Asset Analysis**: Volatility spillovers and regime detection
- **Implications**: Risk management and trading applications

**Key Concepts**: GARCH models, volatility persistence, fat tails, regime switching

## Requirements

```python
# Core dependencies
numpy>=1.20.0
pandas>=1.3.0
matplotlib>=3.4.0
seaborn>=0.11.0
scipy>=1.7.0
```

## Usage

1. Install dependencies:
```bash
pip install numpy pandas matplotlib seaborn scipy
```

2. Launch Jupyter:
```bash
jupyter notebook
```

3. Open any notebook and run cells sequentially

## Methodology Notes

### Data Generation
These notebooks use **synthetic data** to demonstrate methodology:
- Prices follow log-normal dynamics with realistic characteristics
- Market factor model generates cross-sectional correlation
- GARCH processes create volatility clustering
- Cointegrated pairs constructed with embedded relationships

### Backtesting Assumptions
- **Transaction Costs**: 10 bps one-way (configurable)
- **Rebalancing**: Monthly for momentum, dynamic for pairs trading
- **Slippage**: Not modeled (can be added)
- **Shorting**: Assumed freely available

### Performance Metrics
- **Sharpe Ratio**: Excess return / volatility (annualized)
- **Sortino Ratio**: Excess return / downside volatility
- **Calmar Ratio**: Annual return / max drawdown
- **Information Ratio**: Alpha / tracking error

## Extensions & Future Work

- [ ] Add real market data integration (Yahoo Finance, Quandl)
- [ ] Implement EGARCH and GJR-GARCH for asymmetric volatility
- [ ] Add Kalman filter for dynamic hedge ratio estimation
- [ ] Multi-factor model integration (Fama-French)
- [ ] Monte Carlo simulation for strategy robustness testing
