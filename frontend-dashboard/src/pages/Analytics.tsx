import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { Button } from '@/components/ui/button'
import { ArrowUpRight, ArrowDownRight, Download, Calendar } from 'lucide-react'

const trafficSources = [
  { source: "Direct", sessions: 2453, percentage: 40 },
  { source: "Organic Search", sessions: 1842, percentage: 30 },
  { source: "Social Media", sessions: 1224, percentage: 20 },
  { source: "Referral", sessions: 613, percentage: 10 },
]

export default function Analytics() {
  return (
    <div className="space-y-6 pb-10">
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4">
        <div>
          <h2 className="text-3xl font-bold tracking-tight">Analytics</h2>
          <p className="text-muted-foreground mt-1 text-sm md:text-base">Theo dõi hiệu suất và chi tiết các chỉ số nền tảng.</p>
        </div>
        <div className="flex gap-2">
          <Button variant="outline" className="gap-2 shrink-0">
            <Calendar className="w-4 h-4" /> Last 30 Days
          </Button>
          <Button className="shrink-0 gap-2">
            <Download className="w-4 h-4" /> Export Report
          </Button>
        </div>
      </div>

      <div className="grid gap-4 md:grid-cols-3">
        {/* Metric Cards */}
        <Card className="hover:border-primary/40 transition-colors">
          <CardContent className="p-6">
            <div className="flex justify-between items-start">
              <div className="space-y-2">
                <p className="text-sm font-medium text-muted-foreground">Unique Visitors</p>
                <p className="text-4xl font-bold tracking-tight">124.5K</p>
              </div>
              <div className="flex items-center gap-1 text-emerald-500 bg-emerald-500/10 px-2 py-1 rounded-md text-sm font-medium">
                <ArrowUpRight className="w-4 h-4" /> 14.5%
              </div>
            </div>
            <div className="mt-6 h-[60px] w-full flex items-end gap-1">
               {/* Simple sparkline bars */}
               {[40, 70, 45, 90, 65, 85, 100, 50, 75, 45, 80, 60].map((h, i) => (
                 <div key={i} className="flex-1 bg-primary/20 hover:bg-primary transition-colors rounded-t-sm" style={{ height: `${h}%` }}></div>
               ))}
            </div>
          </CardContent>
        </Card>
        
        <Card className="hover:border-destructive/40 transition-colors">
          <CardContent className="p-6">
            <div className="flex justify-between items-start">
              <div className="space-y-2">
                <p className="text-sm font-medium text-muted-foreground">Bounce Rate</p>
                <p className="text-4xl font-bold tracking-tight">42.3%</p>
              </div>
              <div className="flex items-center gap-1 text-red-500 bg-red-500/10 px-2 py-1 rounded-md text-sm font-medium">
                <ArrowDownRight className="w-4 h-4" /> 2.1%
              </div>
            </div>
            <div className="mt-6 h-[60px] w-full flex items-end gap-1">
               {[80, 60, 40, 50, 45, 55, 30, 40, 60, 45, 50, 40].map((h, i) => (
                 <div key={i} className="flex-1 bg-destructive/20 hover:bg-destructive transition-colors rounded-t-sm" style={{ height: `${h}%` }}></div>
               ))}
            </div>
          </CardContent>
        </Card>

        <Card className="hover:border-primary/40 transition-colors">
          <CardContent className="p-6">
            <div className="flex justify-between items-start">
              <div className="space-y-2">
                <p className="text-sm font-medium text-muted-foreground">Session Duration</p>
                <p className="text-4xl font-bold tracking-tight">3m 24s</p>
              </div>
              <div className="flex items-center gap-1 text-emerald-500 bg-emerald-500/10 px-2 py-1 rounded-md text-sm font-medium">
                <ArrowUpRight className="w-4 h-4" /> 5.4%
              </div>
            </div>
             <div className="mt-6 h-[60px] w-full flex items-end gap-1">
               {[20, 30, 50, 40, 60, 45, 70, 50, 65, 80, 70, 90].map((h, i) => (
                 <div key={i} className="flex-1 bg-primary/20 hover:bg-primary transition-colors rounded-t-sm" style={{ height: `${h}%` }}></div>
               ))}
            </div>
          </CardContent>
        </Card>
      </div>

      <div className="grid gap-4 md:grid-cols-3">
        {/* Main CSS Grid Chart */}
        <Card className="md:col-span-2">
          <CardHeader>
            <CardTitle>Audience Overview</CardTitle>
            <p className="text-sm text-muted-foreground">Comparing active users over the last 6 months.</p>
          </CardHeader>
          <CardContent className="px-10 pb-10">
            {/* Fake Area Chart Using CSS Grid */}
            <div className="h-[280px] w-full border-b border-l border-border/50 flex items-end pt-8 pb-0 pl-4 pr-0 gap-2 sm:gap-6 relative mt-4">
              {/* Y Axis labels */}
              <div className="absolute left-0 top-0 bottom-0 flex flex-col justify-between text-[10px] sm:text-xs text-muted-foreground -ml-8 pb-6">
                <span>10k</span>
                <span>7.5k</span>
                <span>5k</span>
                <span>2.5k</span>
                <span></span>
              </div>
              
              {/* Bars */}
              {[ {month: 'Jan', val1: 60, val2: 80}, {month: 'Feb', val1: 40, val2: 60}, {month: 'Mar', val1: 70, val2: 90}, {month: 'Apr', val1: 50, val2: 70}, {month: 'May', val1: 85, val2: 100}, {month: 'Jun', val1: 65, val2: 85}].map((data, i) => (
                <div key={i} className="flex-1 flex flex-col justify-end items-center h-full group relative">
                  <div className="w-full max-w-[48px] flex items-end justify-center gap-0.5 sm:gap-1">
                    <div className="w-1/2 bg-primary/40 rounded-t-[4px] hover:bg-primary/60 transition-colors" style={{ height: `${data.val1}%` }}></div>
                    <div className="w-1/2 bg-primary rounded-t-[4px] hover:brightness-110 shadow-[0_0_15px_rgba(var(--primary),0.3)] transition-colors" style={{ height: `${data.val2}%` }}></div>
                  </div>
                  <span className="text-xs font-medium text-muted-foreground absolute -bottom-8">{data.month}</span>
                </div>
              ))}
            </div>
            
            <div className="flex items-center justify-center gap-6 mt-14">
              <div className="flex items-center gap-2">
                <div className="w-3 h-3 rounded-[3px] bg-primary/40"></div>
                <span className="text-sm font-medium text-muted-foreground">New Users</span>
              </div>
              <div className="flex items-center gap-2">
                <div className="w-3 h-3 rounded-[3px] bg-primary shadow-[0_0_10px_rgba(var(--primary),0.5)]"></div>
                <span className="text-sm font-medium text-muted-foreground">Returning Users</span>
              </div>
            </div>
          </CardContent>
        </Card>

        {/* Donut and Progress Bars */}
        <Card>
          <CardHeader>
            <CardTitle>Traffic Sources</CardTitle>
            <p className="text-sm text-muted-foreground">Where your visitors are coming from.</p>
          </CardHeader>
          <CardContent>
            {/* Fake Donut Chart visual */}
             <div className="py-4 relative flex justify-center items-center h-[180px] mb-6">
                <div className="w-36 h-36 rounded-full border-[14px] border-primary/10 relative shadow-inner">
                  <div className="absolute inset-0 rounded-full border-[14px] border-primary border-t-transparent border-l-transparent rotate-45 group-hover:drop-shadow-[0_0_10px_rgba(255,255,255,0.5)] transition-all"></div>
                  <div className="absolute inset-0 rounded-full border-[14px] border-accent border-b-transparent border-r-transparent -rotate-12"></div>
                </div>
                <div className="absolute flex flex-col items-center justify-center">
                  <span className="text-3xl font-bold tabular-nums tracking-tighter">6.1k</span>
                  <span className="text-xs font-medium text-muted-foreground mt-1 uppercase tracking-wider">Total</span>
                </div>
             </div>

            <div className="space-y-5">
              {trafficSources.map((source, i) => (
                <div key={i} className="space-y-1.5 group cursor-pointer">
                  <div className="flex items-center justify-between text-sm">
                    <span className="font-medium group-hover:text-primary transition-colors">{source.source}</span>
                    <span className="text-muted-foreground tabular-nums">{source.percentage}%</span>
                  </div>
                  <div className="h-2 w-full bg-muted overflow-hidden rounded-full">
                    <div className="h-full bg-primary rounded-full transition-all group-hover:brightness-110" style={{ width: `${source.percentage}%` }}></div>
                  </div>
                </div>
              ))}
            </div>
          </CardContent>
        </Card>
      </div>
    </div>
  )
}
