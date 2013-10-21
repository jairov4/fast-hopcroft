using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using DataAccess;
using OxyPlot;
using OxyPlot.Series;
using OxyPlot.Pdf;
using OxyPlot.Axes;
using PdfSharp.Pdf.IO;
using PdfSharp.Pdf;
using PdfSharp.Drawing;
using System.Windows.Forms;

namespace QAnalyze
{
	class ReportRow501
	{
		public string alg { get; set; }
		public int n { get; set; }
		public int k { get; set; }
		public long t { get; set; }
		public string file { get; set; }
		public int min_st { get; set; }
	}

	public class ReportRow401
	{
		public long states { get; set; }
		public long alpha { get; set; }
		public float d { get; set; }
		public float fd { get; set; }
		public long states_dfa { get; set; }
		public long c_h { get; set; }
		public long t_h { get; set; }
		public long c_i { get; set; }
		public long t_i { get; set; }
		public long c_hi { get; set; }
		public long t_hi { get; set; }
	}

	class Program
	{
		const int CanvasWidth = 640;
		const int CanvasHeight = 400;
		const double MarkerSize = 5.0;
		const double LineThickness = 2.0;

		static void Report401()
		{
			var data = DataTable.New.ReadCsv("report_401.csv");
			var fn_plot_compression_vs_density = "report_401_compression_vs_density.pdf";
			var fn_plot_dfa_states_vs_density = "report_401_dfa_states_vs_density.pdf";

			var rows = data.RowsAs<ReportRow401>();
			var alphas = (from c in rows select c.alpha).Distinct();
			var states = (from c in rows select c.states).Distinct();
			var densities = (from c in rows select c.d).Distinct();
			var finals_densities = (from c in rows select c.fd).Distinct();

			var compression_plot = new PlotModel();
			compression_plot.Title = "States count compression ratio vs density";
			compression_plot.LegendPlacement = LegendPlacement.Outside;
			compression_plot.Axes.Add(new LinearAxis(AxisPosition.Bottom, "Transition function density"));
			compression_plot.Axes.Add(new LinearAxis(AxisPosition.Left, "Ratio"));


			var dfa_states_plot = new PlotModel();
			dfa_states_plot.Title = "Average DFA states vs density";
			dfa_states_plot.LegendPlacement = LegendPlacement.Outside;
			dfa_states_plot.Axes.Add(new LinearAxis(AxisPosition.Bottom, "Transition function density"));
			dfa_states_plot.Axes.Add(new LinearAxis(AxisPosition.Left, "DFA states"));

			var p = from k in alphas from n in states from fd in finals_densities select new { k, n, fd };
			foreach (var item in p)
			{
				var resume = from c in rows
							 where c.states == item.n && c.alpha == item.k && c.fd == item.fd
							 group c by c.d into g
							 let n = g.Average(x => x.states_dfa)
							 let r = g.Average(x => (x.states_dfa - x.c_h) / (float)x.states_dfa)
							 orderby g.Key
							 select new { d = g.Key, n = n, r = r, c = g.Count() };

				var series_ratio = new LineSeries();
				series_ratio.Title = string.Format("k={0}, n={1}, fd={2}", item.k, item.n, item.fd);
				series_ratio.ItemsSource = resume;
				series_ratio.DataFieldX = "d";
				series_ratio.DataFieldY = "r";
				series_ratio.StrokeThickness = LineThickness;

				var series_dfa_states = new LineSeries();
				series_dfa_states.Title = string.Format("NFAs k={0}, n={1} fd={2}", item.k, item.n, item.fd);
				series_dfa_states.ItemsSource = resume;
				series_dfa_states.DataFieldX = "d";
				series_dfa_states.DataFieldY = "n";
				series_dfa_states.StrokeThickness = LineThickness;

				compression_plot.Series.Add(series_ratio);
				dfa_states_plot.Series.Add(series_dfa_states);
			}
			var fn = string.Format(fn_plot_compression_vs_density);
			PdfExporter.Export(compression_plot, fn_plot_compression_vs_density, CanvasWidth, CanvasHeight);
			PdfExporter.Export(dfa_states_plot, fn_plot_dfa_states_vs_density, CanvasWidth, CanvasHeight);
		}

		[STAThread]
		static void Main(string[] args)
		{
			//Directory.SetCurrentDirectory(@"C:\Users\Jairo\Documents\Visual Studio 2012\Projects\fast-hopcroft\build\src\test");
			//Report401();
			Report501();
		}

		private static void Report501()
		{
			var dialog = new OpenFileDialog();
			dialog.Filter = "*.csv|*.csv";
			if (dialog.ShowDialog() != DialogResult.OK) return;

			var dialog2 = new FolderBrowserDialog();
			if (dialog2.ShowDialog() != DialogResult.OK) return;

			var data = DataTable.New.ReadCsv(dialog.FileName);

			var fn_plot_time_vs_states = Path.Combine(dialog2.SelectedPath, "time_vs_states_k{0}.pdf");
			var fn_plot_freq_vs_states = Path.Combine(dialog2.SelectedPath, "freq_vs_states_k{0}.pdf");
			var fn_plot_ratio_vs_states = Path.Combine(dialog2.SelectedPath, "ratio_vs_states_k{0}.pdf");
			var fn_plot_tratio_vs_states = Path.Combine(dialog2.SelectedPath, "timeratio_vs_states_k{0}.pdf");
			var fn_plot_time_vs_alpha = Path.Combine(dialog2.SelectedPath, "time_vs_alpha_n{0}.pdf");
			var fn_plot_freq_vs_alpha = Path.Combine(dialog2.SelectedPath, "freq_vs_alpha_n{0}.pdf");
			var fn_plot_ratio_vs_alpha = Path.Combine(dialog2.SelectedPath, "ratio_vs_alpha_n{0}.pdf");
			var fn_plot_tratio_vs_alpha = Path.Combine(dialog2.SelectedPath, "timeratio_vs_alpha_n{0}.pdf");

			var rows = data.RowsAs<ReportRow501>();
			var alphas = (from c in rows select c.k).Distinct();
			var states = (from c in rows select c.n).Distinct();

			var markerByAlg = new Dictionary<string, MarkerType>();
			markerByAlg.Add("Hopcroft", MarkerType.Circle);
			markerByAlg.Add("Incremental", MarkerType.Square);
			markerByAlg.Add("Hybrid", MarkerType.Diamond);

			var pdf_filenames = new List<string>();

			foreach (var a in alphas)
			{
				var frows = from c in rows where c.k == a select c;
				var algs = (from c in frows select c.alg).Distinct();

				var model_time = new PlotModel();
				model_time.Title = string.Format("Average Execution time vs. States number (k={0})", a);

				var model_freq = new PlotModel();
				model_freq.Title = string.Format("Automata frequency vs. States number (k={0})", a);

				var model_ratio = new PlotModel();
				model_ratio.Title = string.Format("Average Compression Ratio vs. States number (k={0})", a);

				var model_tratio = new PlotModel();
				model_tratio.Title = string.Format("Average Time Execution Ratio vs. States number (k={0})", a);

				foreach (var alg in algs)
				{
					//if (alg == "Hybrid") continue;
					var vals = from c in frows
							   where c.alg == alg
							   group c by c.n into g
							   orderby g.Key
							   select new
							   {
								   n = g.Key,
								   t = g.Average(x => x.t),
								   f = 1.0 / g.Average(x => x.t) * 1.0e9,
								   ratio = g.Average(x => (x.n - x.min_st) / (double)x.n),
							   };
					var markerType = markerByAlg[alg];
					{
						var series = new LineSeries()
						{
							Title = alg,
							ItemsSource = vals,
							DataFieldY = "t",
							MarkerType = markerType
						};
						model_time.Series.Add(series);
					}
					{
						var series = new LineSeries()
						{
							Title = alg,
							ItemsSource = vals,
							DataFieldY = "f",
							MarkerType = markerType
						};
						model_freq.Series.Add(series);
					}
					{
						var series = new LineSeries()
						{
							Title = alg,
							ItemsSource = vals,
							DataFieldY = "ratio",
							MarkerType = markerType
						};
						model_ratio.Series.Add(series);
					}
				}
				// Common Options
				foreach (LineSeries series in model_time.Series.Union(model_freq.Series.Union(model_ratio.Series)))
				{
					series.DataFieldX = "n";
					series.MarkerSize = MarkerSize;
					series.StrokeThickness = LineThickness;
				}
				{
					var pdf_fn = string.Format(fn_plot_time_vs_states, a);
					model_time.Axes.Add(new LinearAxis(AxisPosition.Left, "Time (ns)"));
					model_time.Axes.Add(new LinearAxis(AxisPosition.Bottom, "States"));
					PdfExporter.Export(model_time, pdf_fn, CanvasWidth, CanvasHeight);
					pdf_filenames.Add(pdf_fn);
				}
				{
					var pdf_fn = string.Format(fn_plot_freq_vs_states, a);
					model_freq.Axes.Add(new LinearAxis(AxisPosition.Left, "Frequency (DFAs/s)"));
					model_freq.Axes.Add(new LinearAxis(AxisPosition.Bottom, "States"));
					PdfExporter.Export(model_freq, pdf_fn, CanvasWidth, CanvasHeight);
					pdf_filenames.Add(pdf_fn);
				}
				{
					var pdf_fn = string.Format(fn_plot_ratio_vs_states, a);
					model_ratio.Axes.Add(new LinearAxis(AxisPosition.Left, "Ratio"));
					model_ratio.Axes.Add(new LinearAxis(AxisPosition.Bottom, "States"));
					PdfExporter.Export(model_ratio, pdf_fn, CanvasWidth, CanvasHeight);
					pdf_filenames.Add(pdf_fn);
				}

				var g_hopcroft = from c in rows
								 where c.k == a && c.alg == "Hopcroft"
								 group c by c.n into g
								 select new { n = g.Key, t = g.Average(x => x.t) };
				var g_hybrid = from c in rows
							   where c.k == a && c.alg == "Hybrid"
							   group c by c.n into g
							   select new { n = g.Key, t = g.Average(x => x.t) };
				var g_incremental = from c in rows
									where c.k == a && c.alg == "Incremental"
									group c by c.n into g
									select new { n = g.Key, t = g.Average(x => x.t) };
				var t_hybrid_hopcroft = from c in g_hybrid
										join d in g_hopcroft
										on c.n equals d.n
										orderby c.n
										select new { n = c.n, tratio = c.t / d.t };
				var t_hybrid_incremental = from c in g_hybrid
										   join d in g_incremental
										   on c.n equals d.n
										   orderby c.n
										   select new { n = c.n, tratio = c.t / d.t };
				var t_incremental_hopcroft = from c in g_hopcroft
											 join d in g_incremental
											 on c.n equals d.n
											 orderby c.n
											 select new { n = c.n, tratio = c.t / d.t };
				{
					var series = new LineSeries()
					{
						Title = "Hybrid/Hopcroft",
						ItemsSource = t_hybrid_hopcroft,
						MarkerType = MarkerType.Diamond
					};
					model_tratio.Series.Add(series);
				}
				{
					var series = new LineSeries()
					{
						Title = "Hybrid/Incremental",
						ItemsSource = t_hybrid_incremental,
						MarkerType = MarkerType.Square
					};
					model_tratio.Series.Add(series);
				}
				{
					var series = new LineSeries()
					{
						Title = "Hopcroft/Incremental",
						ItemsSource = t_incremental_hopcroft,
						MarkerType = MarkerType.Circle
					};
					model_tratio.Series.Add(series);
				}
				foreach (LineSeries series in model_tratio.Series)
				{
					series.DataFieldX = "n";
					series.DataFieldY = "tratio";
					series.MarkerSize = MarkerSize;
					series.StrokeThickness = LineThickness;
				}
				{
					var pdf_fn = string.Format(fn_plot_tratio_vs_states, a);
					model_tratio.Axes.Add(new LinearAxis(AxisPosition.Left, "Ratio"));
					model_tratio.Axes.Add(new LinearAxis(AxisPosition.Bottom, "States"));
					PdfExporter.Export(model_tratio, pdf_fn, CanvasWidth, CanvasHeight);
					pdf_filenames.Add(pdf_fn);
				}
			}
			foreach (var n in states)
			{
				var frows = from c in rows where c.n == n select c;
				var algs = (from c in frows select c.alg).Distinct();
				var model_time = new PlotModel();
				model_time.Title = string.Format("Average Execution time vs. Alphabet length (n={0})", n);

				var model_freq = new PlotModel();
				model_freq.Title = string.Format("Automata frequency vs. Alphabet length (n={0})", n);

				var model_ratio = new PlotModel();
				model_ratio.Title = string.Format("Average Compression Ratio vs. Alphabet length (n={0})", n);

				var model_tratio = new PlotModel();
				model_tratio.Title = string.Format("Average Time Execution Ratio vs. Alphabet length (n={0})", n);


				foreach (var alg in algs)
				{
					//if (alg == "Hybrid") continue;
					var vals = from c in frows
							   where c.alg == alg
							   group c by c.k into g
							   orderby g.Key
							   select new
							   {
								   k = g.Key,
								   t = g.Average(x => x.t),
								   f = 1.0 / g.Average(x => x.t) * 1.0e9,
								   ratio = g.Average(x => (x.n - x.min_st) / (double)x.n)
							   };
					var markerType = markerByAlg[alg];
					{
						var series = new LineSeries()
						{
							Title = alg,
							ItemsSource = vals,
							DataFieldY = "t",
							MarkerType = markerType
						};
						model_time.Series.Add(series);
					}
					{
						var series = new LineSeries()
						{
							Title = alg,
							ItemsSource = vals,
							DataFieldY = "f",
							MarkerType = markerType
						};
						model_freq.Series.Add(series);
					}
					{
						var series = new LineSeries()
						{
							Title = alg,
							ItemsSource = vals,
							DataFieldY = "ratio",
							MarkerType = markerType
						};
						model_ratio.Series.Add(series);
					}
				}
				// Common options
				foreach (LineSeries series in model_time.Series.Union(model_freq.Series.Union(model_ratio.Series)))
				{
					series.DataFieldX = "k";
					series.MarkerSize = MarkerSize;
					series.StrokeThickness = LineThickness;
				}
				{
					var pdf_fn = string.Format(fn_plot_time_vs_alpha, n);
					model_time.Axes.Add(new LinearAxis(AxisPosition.Left, "Time (ns)"));
					model_time.Axes.Add(new LinearAxis(AxisPosition.Bottom, "Symbols"));
					PdfExporter.Export(model_time, pdf_fn, CanvasWidth, CanvasHeight);
					pdf_filenames.Add(pdf_fn);
				}
				{
					var pdf_fn = string.Format(fn_plot_freq_vs_alpha, n);
					model_freq.Axes.Add(new LinearAxis(AxisPosition.Left, "Frequency (DFAs/s)"));
					model_freq.Axes.Add(new LinearAxis(AxisPosition.Bottom, "Symbols"));
					PdfExporter.Export(model_freq, pdf_fn, CanvasWidth, CanvasHeight);
					pdf_filenames.Add(pdf_fn);
				}
				{
					var pdf_fn = string.Format(fn_plot_ratio_vs_alpha, n);
					model_freq.Axes.Add(new LinearAxis(AxisPosition.Left, "Ratio"));
					model_freq.Axes.Add(new LinearAxis(AxisPosition.Bottom, "Symbols"));
					PdfExporter.Export(model_ratio, pdf_fn, CanvasWidth, CanvasHeight);
					pdf_filenames.Add(pdf_fn);
				}

				var g_hopcroft = from c in rows
								 where c.n == n && c.alg == "Hopcroft"
								 group c by c.k into g
								 select new { k = g.Key, t = g.Average(x => x.t) };
				var g_hybrid = from c in rows
							   where c.n == n && c.alg == "Hybrid"
							   group c by c.k into g
							   select new { k = g.Key, t = g.Average(x => x.t) };
				var g_incremental = from c in rows
									where c.n == n && c.alg == "Incremental"
									group c by c.k into g
									select new { k = g.Key, t = g.Average(x => x.t) };
				var t_hybrid_hopcroft = from c in g_hybrid
										join d in g_hopcroft
										on c.k equals d.k
										orderby c.k
										select new { k = c.k, tratio = c.t / d.t };
				var t_hybrid_incremental = from c in g_hybrid
										   join d in g_incremental
										   on c.k equals d.k
										   orderby c.k
										   select new { k = c.k, tratio = c.t / d.t };
				var t_incremental_hopcroft = from c in g_hopcroft
											 join d in g_incremental
											 on c.k equals d.k
											 orderby c.k
											 select new { k = c.k, tratio = c.t / d.t };
				{
					var series = new LineSeries()
					{
						Title = "Hybrid/Hopcroft",
						ItemsSource = t_hybrid_hopcroft,
						MarkerType = MarkerType.Diamond
					};
					model_tratio.Series.Add(series);
				}
				{
					var series = new LineSeries()
					{
						Title = "Hybrid/Incremental",
						ItemsSource = t_hybrid_incremental,
						MarkerType = MarkerType.Square
					};
					model_tratio.Series.Add(series);
				}
				{
					var series = new LineSeries()
					{
						Title = "Hopcroft/Incremental",
						ItemsSource = t_incremental_hopcroft,
						MarkerType = MarkerType.Circle
					};
					model_tratio.Series.Add(series);
				}
				foreach (LineSeries series in model_tratio.Series)
				{
					series.DataFieldX = "k";
					series.DataFieldY = "tratio";
					series.MarkerSize = MarkerSize;
					series.StrokeThickness = LineThickness;
				}
				{
					var pdf_fn = string.Format(fn_plot_tratio_vs_alpha, n);
					model_tratio.Axes.Add(new LinearAxis(AxisPosition.Left, "Ratio"));
					model_tratio.Axes.Add(new LinearAxis(AxisPosition.Bottom, "Symbols"));
					PdfExporter.Export(model_tratio, pdf_fn, CanvasWidth, CanvasHeight);
					pdf_filenames.Add(pdf_fn);
				}
			}
			pdf_filenames.Sort();
			MergePdf(pdf_filenames, Path.Combine(dialog2.SelectedPath, "charts_combined.pdf"));
		}

		static void MergePdf(IEnumerable<string> documents, string fnoutpdf)
		{
			var opdf = new PdfDocument();
			foreach (var item in documents)
			{
				var ipdf = PdfReader.Open(item, PdfDocumentOpenMode.Import);
				var page = ipdf.Pages[0];
				opdf.AddPage(page);
			}
			opdf.Save(fnoutpdf);
		}
	}

}
