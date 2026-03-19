import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { Button } from '@/components/ui/button'
import { Search, Filter, Plus, MoreHorizontal } from 'lucide-react'

const customers = [
  { id: '1', name: 'Olivia Martin', email: 'olivia.martin@email.com', role: 'Admin', status: 'Active', lastActive: '2 mins ago' },
  { id: '2', name: 'Jackson Lee', email: 'jackson.lee@email.com', role: 'User', status: 'Offline', lastActive: '1 hr ago' },
  { id: '3', name: 'Isabella Nguyen', email: 'isabella.nguyen@email.com', role: 'User', status: 'Active', lastActive: '5 mins ago' },
  { id: '4', name: 'William Kim', email: 'will@email.com', role: 'Editor', status: 'Offline', lastActive: 'Yesterday' },
  { id: '5', name: 'Sofia Davis', email: 'sofia.davis@email.com', role: 'User', status: 'Active', lastActive: '10 mins ago' },
  { id: '6', name: 'Ethan Hunt', email: 'ethan.h@email.com', role: 'User', status: 'Offline', lastActive: '2 days ago' },
  { id: '7', name: 'Emma Watson', email: 'emma.w@email.com', role: 'User', status: 'Active', lastActive: 'Just now' },
]

export default function Customers() {
  return (
    <div className="space-y-6 pb-10">
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4">
        <div>
          <h2 className="text-3xl font-bold tracking-tight">Customers</h2>
          <p className="text-muted-foreground mt-1 text-sm md:text-base">Quản lý tài khoản khách hàng và phân quyền.</p>
        </div>
        <Button className="shrink-0 gap-2">
          <Plus className="w-4 h-4" /> Add Customer
        </Button>
      </div>

      <Card>
        <CardHeader className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 py-4">
          <CardTitle className="text-base font-medium">All Customers</CardTitle>
          <div className="flex flex-col sm:flex-row items-start sm:items-center gap-2 w-full sm:w-auto">
            <div className="relative w-full sm:w-[250px]">
              <Search className="absolute left-2.5 top-1/2 -translate-y-1/2 w-4 h-4 text-muted-foreground" />
              <input 
                type="text" 
                placeholder="Search customers..." 
                className="w-full bg-background border border-border rounded-md px-3 py-1.5 pl-9 text-sm focus:outline-none focus:ring-2 focus:ring-primary/20 transition-all font-sans"
              />
            </div>
            <Button variant="outline" size="sm" className="gap-2 shrink-0">
              <Filter className="w-4 h-4" /> Filter
            </Button>
          </div>
        </CardHeader>
        <CardContent className="p-0">
          <div className="overflow-x-auto">
            <table className="w-full text-sm text-left whitespace-nowrap">
              <thead className="text-xs text-muted-foreground bg-muted/50 border-y border-border/50 uppercase">
                <tr>
                  <th className="px-6 py-3 font-medium">Name</th>
                  <th className="px-6 py-3 font-medium">Role</th>
                  <th className="px-6 py-3 font-medium">Status</th>
                  <th className="px-6 py-3 font-medium">Last Active</th>
                  <th className="px-6 py-3 font-medium text-right">Actions</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-border/50">
                {customers.map((customer) => (
                  <tr key={customer.id} className="hover:bg-muted/20 transition-colors group">
                    <td className="px-6 py-4">
                      <div className="flex items-center gap-3">
                        <div className="w-8 h-8 rounded-full bg-primary/10 flex items-center justify-center font-medium text-primary text-xs uppercase group-hover:bg-primary group-hover:text-primary-foreground transition-colors shrink-0">
                          {customer.name.substring(0,2)}
                        </div>
                        <div className="min-w-0">
                          <p className="font-medium text-foreground truncate">{customer.name}</p>
                          <p className="text-xs text-muted-foreground truncate">{customer.email}</p>
                        </div>
                      </div>
                    </td>
                    <td className="px-6 py-4">
                      <span className="inline-flex items-center px-2 py-1 rounded-md bg-secondary text-secondary-foreground text-xs font-medium">
                        {customer.role}
                      </span>
                    </td>
                    <td className="px-6 py-4">
                      <div className="flex items-center gap-2 text-muted-foreground">
                        <div className={`w-2 h-2 rounded-full ${customer.status === 'Active' ? 'bg-emerald-500' : 'bg-muted-foreground/30'}`}></div>
                        <span>{customer.status}</span>
                      </div>
                    </td>
                    <td className="px-6 py-4 text-muted-foreground text-xs">
                      {customer.lastActive}
                    </td>
                    <td className="px-6 py-4 text-right">
                      <Button variant="ghost" size="icon-sm" className="text-muted-foreground hover:text-foreground">
                        <MoreHorizontal className="w-4 h-4" />
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
