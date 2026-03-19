import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { DollarSign, Users, CreditCard, Activity } from 'lucide-react'
import { useAutoAnimate } from '@formkit/auto-animate/react'

const stats = [
  { title: 'Total Revenue', value: '$45,231.89', subtitle: '+20.1% from last month', icon: DollarSign },
  { title: 'Subscriptions', value: '+2,350', subtitle: '+180.1% from last month', icon: Users },
  { title: 'Sales', value: '+12,234', subtitle: '+19% from last month', icon: CreditCard },
  { title: 'Active Now', value: '+573', subtitle: '+201 since last hour', icon: Activity },
]

const recentSales = [
  { name: 'Olivia Martin', email: 'olivia.martin@email.com', amount: '+$1,999.00' },
  { name: 'Jackson Lee', email: 'jackson.lee@email.com', amount: '+$39.00' },
  { name: 'Isabella Nguyen', email: 'isabella.nguyen@email.com', amount: '+$299.00' },
  { name: 'William Kim', email: 'will@email.com', amount: '+$99.00' },
  { name: 'Sofia Davis', email: 'sofia.davis@email.com', amount: '+$39.00' },
]

export default function Dashboard() {
  const [listRef] = useAutoAnimate<HTMLUListElement>()

  return (
    <div className="space-y-8 pb-10">
      <div>
        <h2 className="text-3xl font-bold tracking-tight">Dashboard</h2>
        <p className="text-muted-foreground mt-1 text-sm md:text-base">Quản lý tổng quan hoạt động kinh doanh của bạn.</p>
      </div>

      <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-4">
        {stats.map((stat, index) => (
          <Card key={index} className="hover:border-primary/40 transition-colors group cursor-pointer">
            <CardHeader className="flex flex-row items-center justify-between pb-2 space-y-0">
              <CardTitle className="text-sm font-medium text-muted-foreground group-hover:text-foreground transition-colors">{stat.title}</CardTitle>
              <stat.icon className="w-4 h-4 text-muted-foreground group-hover:text-primary transition-colors" />
            </CardHeader>
            <CardContent>
              <div className="text-2xl font-bold">{stat.value}</div>
              <p className="text-xs text-muted-foreground mt-1">{stat.subtitle}</p>
            </CardContent>
          </Card>
        ))}
      </div>

      <div className="grid gap-4 grid-cols-1 md:grid-cols-2 lg:grid-cols-7">
        <Card className="col-span-1 md:col-span-2 lg:col-span-4 flex flex-col">
          <CardHeader>
            <CardTitle>Overview</CardTitle>
          </CardHeader>
          <CardContent className="flex-1 flex items-center justify-center text-muted-foreground border-t border-border/50 bg-muted/20 rounded-b-xl mx-6 mb-6 mt-2 min-h-[300px]">
            <p className="text-sm">Chỗ này dành cho biểu đồ (Chart Placeholder)</p>
          </CardContent>
        </Card>

        <Card className="col-span-1 md:col-span-2 lg:col-span-3">
          <CardHeader>
            <CardTitle>Recent Sales</CardTitle>
            <p className="text-sm text-muted-foreground font-normal">You made 265 sales this month.</p>
          </CardHeader>
          <CardContent>
            <ul ref={listRef} className="space-y-6 mt-2">
              {recentSales.map((sale, i) => (
                <li key={i} className="flex items-center justify-between group cursor-pointer hover:bg-muted/30 p-2 -mx-2 rounded-lg transition-colors gap-3">
                  <div className="flex items-center space-x-4 min-w-0 flex-1">
                    <div className="w-10 h-10 shrink-0 rounded-full bg-primary/10 flex items-center justify-center font-medium text-primary text-sm uppercase group-hover:bg-primary group-hover:text-primary-foreground transition-colors">
                      {sale.name.substring(0,2)}
                    </div>
                    <div className="min-w-0 overflow-hidden flex-1">
                      <p className="text-sm font-medium leading-none group-hover:text-primary transition-colors truncate">{sale.name}</p>
                      <p className="text-sm text-muted-foreground mt-1 truncate">{sale.email}</p>
                    </div>
                  </div>
                  <div className="font-medium text-sm whitespace-nowrap pl-2">{sale.amount}</div>
                </li>
              ))}
            </ul>
          </CardContent>
        </Card>
      </div>
    </div>
  )
}
