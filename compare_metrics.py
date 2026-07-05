import pandas as pd
import pandas as pd
import numpy as np
from sklearn.metrics import mean_squared_error, mean_absolute_error, r2_score
import warnings
warnings.filterwarnings('ignore')

output_df = pd.read_csv('output.csv')
ground_truth_df = pd.read_csv('ground_truth.csv')

print(f"Output CSV shape: {output_df.shape}")
print(f"Ground Truth CSV shape: {ground_truth_df.shape}")
print("\n" + "="*80 + "\n")

merge_keys = ['time', 'ts_ns', 'symbol']

merged_df = output_df.merge(
    ground_truth_df, 
    on=merge_keys, 
    how='inner', 
    suffixes=('_output', '_gt')
)

print(f"Number of matching samples: {len(merged_df)}")

sample_size = min(2000, len(merged_df))
sampled_df = merged_df.sample(n=sample_size, random_state=42) if len(merged_df) > 0 else merged_df

print(f"Number of samples analyzed: {len(sampled_df)}")
print("\n" + "="*80 + "\n")

columns_to_compare = [
    'ask1_price', 'ask1_qty', 'ask1_orders',
    'ask2_price', 'ask2_qty', 'ask2_orders'
]

results = {}
all_metrics = []

print("DETAILED METRICS FOR EACH COLUMN")
print("="*80)

for col in columns_to_compare:
    col_output = f"{col}_output" if f"{col}_output" in sampled_df.columns else col
    col_gt = f"{col}_gt" if f"{col}_gt" in sampled_df.columns else col

    if col_output in sampled_df.columns and col_gt in sampled_df.columns:
        output_vals = sampled_df[col_output].values
        gt_vals = sampled_df[col_gt].values

        mask = ~(np.isnan(output_vals) | np.isnan(gt_vals))
        output_vals = output_vals[mask]
        gt_vals = gt_vals[mask]

        if len(output_vals) == 0:
            print(f"\n{col}: No valid data to compare")
            continue

        diff = output_vals - gt_vals
        abs_diff = np.abs(diff)

        mape = np.mean(np.abs(diff) / (np.abs(gt_vals) + 1e-10)) * 100

        rmse = np.sqrt(mean_squared_error(gt_vals, output_vals))
        mae = mean_absolute_error(gt_vals, output_vals)

        gt_range = gt_vals.max() - gt_vals.min()
        nrmse = (rmse / gt_range * 100) if gt_range > 0 else 0

        r2 = r2_score(gt_vals, output_vals)

        correlation = np.corrcoef(output_vals, gt_vals)[0, 1]

        mdape = np.median(np.abs(diff) / (np.abs(gt_vals) + 1e-10)) * 100

        tolerance_5pct = np.sum(np.abs(diff) / (np.abs(gt_vals) + 1e-10) <= 0.05) / len(diff) * 100
        tolerance_10pct = np.sum(np.abs(diff) / (np.abs(gt_vals) + 1e-10) <= 0.10) / len(diff) * 100
        tolerance_20pct = np.sum(np.abs(diff) / (np.abs(gt_vals) + 1e-10) <= 0.20) / len(diff) * 100

        results[col] = {
            'mae': mae,
            'rmse': rmse,
            'nrmse': nrmse,
            'mape': mape,
            'mdape': mdape,
            'r2': r2,
            'correlation': correlation,
            'tolerance_5pct': tolerance_5pct,
            'tolerance_10pct': tolerance_10pct,
            'tolerance_20pct': tolerance_20pct,
            'mean_diff': diff.mean(),
            'std_diff': diff.std(),
            'median_abs_diff': np.median(abs_diff),
            'max_abs_diff': abs_diff.max(),
            'min_gt': gt_vals.min(),
            'max_gt': gt_vals.max(),
            'mean_gt': gt_vals.mean()
        }

        print(f"\n{'-'*80}")
        print(f"Column: {col}")
        print(f"{'-'*80}")
        print(f"Ground Truth Stats:")
        print(f"  Range: [{gt_vals.min():.2f}, {gt_vals.max():.2f}]")
        print(f"  Mean: {gt_vals.mean():.2f}")
        print(f"  Std: {gt_vals.std():.2f}")
        print(f"\nError Metrics:")
        print(f"  MAE (Mean Absolute Error):           {mae:.4f}")
        print(f"  RMSE (Root Mean Squared Error):      {rmse:.4f}")
        print(f"  NRMSE (Normalized RMSE):             {nrmse:.2f}%")
        print(f"  MAPE (Mean Absolute % Error):        {mape:.2f}%")
        print(f"  MdAPE (Median Absolute % Error):     {mdape:.2f}%")
        print(f"\nQuality Metrics:")
        print(f"  R-Squared Score (1.0 is perfect):    {r2:.4f}")
        print(f"  Correlation (1.0 is perfect):        {correlation:.4f}")
        print(f"\nTolerance Analysis:")
        print(f"  Within 5% error:                     {tolerance_5pct:.2f}%")
        print(f"  Within 10% error:                    {tolerance_10pct:.2f}%")
        print(f"  Within 20% error:                    {tolerance_20pct:.2f}%")
        print(f"\nDifference Stats:")
        print(f"  Mean difference (bias):              {diff.mean():.4f}")
        print(f"  Median absolute difference:          {np.median(abs_diff):.4f}")
        print(f"  Max absolute difference:             {abs_diff.max():.4f}")
        print(f"  Std of difference:                   {diff.std():.4f}")

        all_metrics.append({
            'column': col,
            'mae': mae,
            'rmse': rmse,
            'nrmse': nrmse,
            'mape': mape,
            'r2': r2,
            'correlation': correlation
        })

print(f"\n\n{'='*80}")
print("OVERALL SUMMARY")
print("="*80)

if all_metrics:
    df_metrics = pd.DataFrame(all_metrics)

    print("\nComparative Table:")
    print(df_metrics.to_string(index=False))

    print(f"\n\nAggregate Scores:")
    print(f"  Average MAPE across all columns:      {df_metrics['mape'].mean():.2f}%")
    print(f"  Average NRMSE across all columns:     {df_metrics['nrmse'].mean():.2f}%")
    print(f"  Average R-Squared Score:              {df_metrics['r2'].mean():.4f}")
    print(f"  Average Correlation:                  {df_metrics['correlation'].mean():.4f}")

    print(f"\n\nInterpretation Guide:")
    print(f"  MAPE < 10%:   Excellent accuracy")
    print(f"  MAPE 10-20%:  Good accuracy")
    print(f"  MAPE 20-50%:  Moderate accuracy")
    print(f"  MAPE > 50%:   Poor accuracy")
    print(f"\n  R-Squared > 0.9:     Excellent fit")
    print(f"  R-Squared 0.7-0.9:   Good fit")
    print(f"  R-Squared 0.5-0.7:   Moderate fit")
    print(f"  R-Squared < 0.5:     Poor fit")

    print("\n" + "="*80)
    print("KEY RECOMMENDATIONS:")
    print("="*80)

    for idx, row in df_metrics.iterrows():
        col = row['column']
        if row['mape'] > 50:
            print(f"WARNING {col}: HIGH ERROR (MAPE={row['mape']:.1f}%) - Needs significant improvement")
        elif row['mape'] > 20:
            print(f"ALERT {col}: MODERATE ERROR (MAPE={row['mape']:.1f}%) - Could be improved")
        elif row['mape'] > 10:
            print(f"OK {col}: GOOD (MAPE={row['mape']:.1f}%)")
        else:
            print(f"EXCELLENT {col}: EXCELLENT (MAPE={row['mape']:.1f}%)")
