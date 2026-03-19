import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { Button } from '@/components/ui/button'
import { CreditCard } from 'lucide-react'

export default function BillingTab() {
  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
      {/* Current Plan Card */}
      <Card>
        <CardHeader>
          <CardTitle>Subscription Plan</CardTitle>
          <p className="text-sm text-muted-foreground">Manage your subscription and billing details.</p>
        </CardHeader>
        <CardContent>
          <div className="flex flex-col sm:flex-row justify-between sm:items-center gap-4 p-4 border border-primary/20 bg-primary/5 rounded-lg transition-colors hover:border-primary/40">
            <div>
              <div className="flex items-center gap-2">
                <h3 className="text-lg font-bold">Pro Plan</h3>
                <span className="px-2 py-0.5 rounded-full bg-primary text-primary-foreground text-[10px] font-bold uppercase tracking-wider shadow-[0_0_10px_rgba(var(--primary),0.3)]">Active</span>
              </div>
              <p className="text-sm text-muted-foreground mt-1">Next payment of $24.00 on Dec 1, 2026.</p>
            </div>
            <div className="flex gap-2 w-full sm:w-auto mt-2 sm:mt-0">
              <Button variant="outline" className="flex-1 sm:flex-none">Cancel Plan</Button>
              <Button className="flex-1 sm:flex-none">Upgrade</Button>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Payment Method Card */}
      <Card>
        <CardHeader>
          <CardTitle>Payment Method</CardTitle>
          <p className="text-sm text-muted-foreground">Update your credit card information.</p>
        </CardHeader>
        <CardContent className="space-y-4">
          <div className="flex items-center justify-between p-4 border rounded-lg bg-card transition-colors hover:bg-muted/10">
            <div className="flex items-center gap-4">
              <div className="w-12 h-8 rounded bg-gradient-to-tr from-slate-200 to-slate-100 flex items-center justify-center shrink-0 border border-border shadow-sm">
                <span className="text-[10px] font-black text-blue-900 italic tracking-tighter">VISA</span>
              </div>
              <div className="min-w-0">
                <p className="text-sm font-medium">Visa ending in 4242</p>
                <p className="text-xs text-muted-foreground">Expires 12/28</p>
              </div>
            </div>
            <Button variant="ghost" size="sm" className="text-muted-foreground hover:text-foreground shrink-0 border border-transparent hover:border-border">Edit</Button>
          </div>
          <Button variant="outline" className="w-full gap-2 border-dashed hover:border-solid hover:bg-muted/50 transition-all font-medium">
            <CreditCard className="w-4 h-4" /> Add Payment Method
          </Button>
        </CardContent>
      </Card>

      {/* Invoices History */}
      <Card>
        <CardHeader>
          <CardTitle>Billing History</CardTitle>
          <p className="text-sm text-muted-foreground">Download your previous invoices.</p>
        </CardHeader>
        <CardContent className="p-0">
          <div className="overflow-x-auto">
             <table className="w-full text-sm text-left whitespace-nowrap">
              <thead className="bg-muted/50 text-muted-foreground border-y border-border/50 text-xs uppercase tracking-wider">
                <tr>
                  <th className="px-6 py-3 font-medium">Date</th>
                  <th className="px-6 py-3 font-medium">Amount</th>
                  <th className="px-6 py-3 font-medium">Status</th>
                  <th className="px-6 py-3 font-medium text-right">Invoice</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-border/50">
                {['Nov 1, 2026', 'Oct 1, 2026', 'Sep 1, 2026', 'Aug 1, 2026'].map((date, i) => (
                  <tr key={i} className="hover:bg-muted/20 transition-colors group">
                    <td className="px-6 py-4">{date}</td>
                    <td className="px-6 py-4 font-medium">$24.00</td>
                    <td className="px-6 py-4">
                      <span className="text-emerald-500 bg-emerald-500/10 px-2 py-1 rounded-md text-xs font-semibold group-hover:bg-emerald-500/20 transition-colors">Paid</span>
                    </td>
                    <td className="px-6 py-4 text-right">
                      <Button variant="link" size="sm" className="h-auto p-0 text-primary hover:underline hover:text-primary/80 decoration-primary/50 underline-offset-4">
                        Download PDF
                      </Button>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </CardContent>
      </Card>
    </div>
  )
}
